#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameRoom.h"
#include "Player.h"
#include "GameDB.h"
#include "DBManager.h"
#include "PlayerManager.h"
#include "HomeRoom.h"
#include "HomeRoomManager.h"
#include "GameGlobal.h"

bool ClientPacketHandler::OnHandle_C_ENTER(GameSessionRef session, const Protocol::CEnter& pkt)
{
    const uint64      playerId = pkt.player().player_id();
    const std::string username = pkt.player().username();

    GDBManager->Async([session, playerId, username](DBConnection& conn)
    {
        // DB 비동기 워커 스레드에서 실행
        PlayerRef player = GameDB::LoadOrCreate(conn, playerId, username);
        auto quests      = GameDB::LoadDailyQuests(conn, playerId);

        // 연결이 끊겼으면 처리 중단
        if (!session->IsConnected()) return;

        // 세션에 플레이어 등록 + 전역 매핑
        session->SetPlayer(player);
        GPlayerManager->Register(player->GetPlayerId(), session);

        // 자기 HomeRoom 생성 후 입장
        HomeRoomRef room = GHomeRoomManager->Create(player->GetPlayerId(), session);
        session->SetCurrentRoom(room);

        GGameRoom->Enter(session);

        // S_ENTER 응답
        {
            Protocol::SEnter res;
            res.set_success(true);
            session->Send(MakeSendBuffer<Protocol::S_ENTER>(res));
        }

        // S_PLAYER_DATA 응답 (플레이어 상세 정보 + 퀘스트)
        {
            Protocol::SPlayerData res;

            auto* info = res.mutable_info();
            info->set_player_id(player->GetPlayerId());
            info->set_username(player->GetUsername());
            info->set_level(player->GetLevel());
            info->set_exp(player->GetExp());
            info->set_points(player->GetPoints());

            auto* stats = res.mutable_stats();
            stats->set_hygiene(player->GetStats().hygiene);
            stats->set_toxicity(player->GetStats().toxicity);
            stats->set_stability(player->GetStats().stability);
            stats->set_comfort(player->GetStats().comfort);
            stats->set_family_hp(player->GetStats().family_hp);

            for (auto& q : quests)
                *res.add_daily_quests() = q;

            session->Send(MakeSendBuffer<Protocol::S_PLAYER_DATA>(res));
        }
    });

    return true;
}

bool ClientPacketHandler::OnHandle_C_COMPLETE_QUEST(GameSessionRef session, const Protocol::CCompleteQuest& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (!player) return false;

    const uint64 playerId = player->GetPlayerId();
    const int32  questId  = pkt.quest_id();

    GDBManager->Async([session, player, playerId, questId](DBConnection& conn)
    {
        // ── 1. 퀘스트 완료 처리 ───────────────────────────────────────────────
        if (!GameDB::CompleteQuest(conn, playerId, questId))
        {
            // 이미 완료됐거나 오늘 퀘스트가 아닌 경우
            if (!session->IsConnected()) return;
            Protocol::SQuestResult res;
            res.set_success (false);
            res.set_quest_id(questId);
            session->Send(MakeSendBuffer<Protocol::S_QUEST_RESULT>(res));
            return;
        }

        // ── 2. 보상 조회 ──────────────────────────────────────────────────────
        GameDB::QuestReward reward = GameDB::GetQuestReward(conn, questId);

        // ── 3. 보상 적용 (메모리) ─────────────────────────────────────────────
        LevelUpResult lvResult = player->AddExp(reward.expGained);
        player->AddPoints(reward.ptsGained);
        if (reward.statType >= 0)
            player->ApplyStat(reward.statType, reward.statValue);

        // ── 4. 변경된 수치 즉시 DB 저장 ──────────────────────────────────────
        GameDB::SavePlayer(conn, player);

        // ── 5. 결과 전송 ──────────────────────────────────────────────────────
        if (!session->IsConnected()) return;

        Protocol::SQuestResult res;
        res.set_success   (true);
        res.set_quest_id  (questId);
        res.set_exp_gained(reward.expGained);
        res.set_pts_gained(reward.ptsGained);
        res.set_stat_type (reward.statType);
        res.set_stat_value(reward.statValue);
        res.set_leveled_up(lvResult.leveledUp);
        res.set_new_level (lvResult.newLevel);
        session->Send(MakeSendBuffer<Protocol::S_QUEST_RESULT>(res));
    });

    return true;
}

bool ClientPacketHandler::OnHandle_C_ROOM_CHAT(GameSessionRef session, const Protocol::CRoomChat& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (!player) return false;

    HomeRoomRef room = session->GetCurrentRoom();
    if (!room) return false;

    Protocol::SRoomChat res;
    res.set_from_id  (player->GetPlayerId());
    res.set_from_name(player->GetUsername());
    res.set_message  (pkt.message());

    // 발신자 제외 브로드캐스트
    room->BroadcastExcept(MakeSendBuffer<Protocol::S_ROOM_CHAT>(res), session);
    return true;
}

bool ClientPacketHandler::OnHandle_C_WHISPER(GameSessionRef session, const Protocol::CWhisper& pkt)
{
    PlayerRef sender = session->GetPlayer();
    if (!sender) return false;

    GameSessionRef target = GPlayerManager->Find(pkt.target_id());
    if (!target)
    {
        Protocol::SWhisperFailed fail;
        fail.set_reason(0);   // 0 = 오프라인
        session->Send(MakeSendBuffer<Protocol::S_WHISPER_FAILED>(fail));
        return true;
    }

    Protocol::SWhisper res;
    res.set_from_id  (sender->GetPlayerId());
    res.set_from_name(sender->GetUsername());
    res.set_message  (pkt.message());
    target->Send(MakeSendBuffer<Protocol::S_WHISPER>(res));
    return true;
}

bool ClientPacketHandler::OnHandle_C_INVITE_FRIEND(GameSessionRef session, const Protocol::CInviteFriend& pkt)
{
    PlayerRef sender = session->GetPlayer();
    if (!sender) return false;

    GameSessionRef target = GPlayerManager->Find(pkt.target_id());
    if (!target)
    {
        // 오프라인 → 클라이언트에 실패 알림 (SInviteDeclined 재활용)
        Protocol::SInviteDeclined fail;
        fail.set_target_id(pkt.target_id());
        session->Send(MakeSendBuffer<Protocol::S_INVITE_DECLINED>(fail));
        return true;
    }

    Protocol::SInviteRequest req;
    req.set_from_id  (sender->GetPlayerId());
    req.set_from_name(sender->GetUsername());
    target->Send(MakeSendBuffer<Protocol::S_INVITE_REQUEST>(req));
    return true;
}

bool ClientPacketHandler::OnHandle_C_INVITE_RESPONSE(GameSessionRef session, const Protocol::CInviteResponse& pkt)
{
    if (!pkt.accept())
    {
        // 거절 → 초대한 사람에게 알림
        GameSessionRef inviter = GPlayerManager->Find(pkt.from_id());
        if (inviter)
        {
            Protocol::SInviteDeclined declined;
            PlayerRef p = session->GetPlayer();
            declined.set_target_id(p ? p->GetPlayerId() : 0);
            inviter->Send(MakeSendBuffer<Protocol::S_INVITE_DECLINED>(declined));
        }
        return true;
    }

    // 수락 → 초대한 사람의 HomeRoom 입장
    HomeRoomRef room = GHomeRoomManager->Find(pkt.from_id());
    if (!room) return true;   // 이미 로그아웃한 경우

    // 기존 방에서 나가기
    HomeRoomRef prevRoom = session->GetCurrentRoom();
    if (prevRoom && prevRoom != room)
        prevRoom->LeaveVisitor(session);

    session->SetCurrentRoom(room);
    room->EnterVisitor(session);
    return true;
}

bool ClientPacketHandler::OnHandle_C_VISIT_REQUEST(GameSessionRef session, const Protocol::CVisitRequest& pkt)
{
    PlayerRef visitor = session->GetPlayer();
    if (!visitor) return false;

    GameSessionRef target = GPlayerManager->Find(pkt.target_id());
    if (!target) return true;   // 오프라인

    Protocol::SVisitRequest req;
    req.set_from_id  (visitor->GetPlayerId());
    req.set_from_name(visitor->GetUsername());
    target->Send(MakeSendBuffer<Protocol::S_VISIT_REQUEST>(req));
    return true;
}

bool ClientPacketHandler::OnHandle_C_VISIT_RESPONSE(GameSessionRef session, const Protocol::CVisitResponse& pkt)
{
    if (!pkt.accept())
    {
        GameSessionRef requester = GPlayerManager->Find(pkt.from_id());
        if (requester)
        {
            Protocol::SVisitDeclined declined;
            requester->Send(MakeSendBuffer<Protocol::S_VISIT_DECLINED>(declined));
        }
        return true;
    }

    // 수락 → 방문 요청자를 내 HomeRoom으로 입장
    PlayerRef owner = session->GetPlayer();
    if (!owner) return false;

    GameSessionRef requester = GPlayerManager->Find(pkt.from_id());
    if (!requester) return true;

    HomeRoomRef room = GHomeRoomManager->Find(owner->GetPlayerId());
    if (!room) return true;

    HomeRoomRef prevRoom = requester->GetCurrentRoom();
    if (prevRoom && prevRoom != room)
        prevRoom->LeaveVisitor(requester);

    requester->SetCurrentRoom(room);
    room->EnterVisitor(requester);
    return true;
}

bool ClientPacketHandler::OnHandle_C_KICK_PLAYER(GameSessionRef session, const Protocol::CKickPlayer& pkt)
{
    PlayerRef owner = session->GetPlayer();
    if (!owner) return false;

    // 자기 HomeRoom의 주인만 강퇴 가능
    HomeRoomRef room = GHomeRoomManager->Find(owner->GetPlayerId());
    if (!room) return false;

    // 강퇴 대상의 CurrentRoom을 자기 HomeRoom으로 되돌리기
    GameSessionRef target = GPlayerManager->Find(pkt.target_id());
    if (target)
    {
        HomeRoomRef targetOwnRoom = GHomeRoomManager->Find(pkt.target_id());
        target->SetCurrentRoom(targetOwnRoom);
    }

    room->Kick(pkt.target_id());
    return true;
}

bool ClientPacketHandler::OnHandle_C_GET_FRIEND_LIST(GameSessionRef session, const Protocol::CGetFriendList& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (!player) return false;

    const uint64 playerId = player->GetPlayerId();

    GDBManager->Async([session, playerId](DBConnection& conn)
    {
        auto friends = GameDB::LoadFriendList(conn, playerId);

        if (!session->IsConnected()) return;

        Protocol::SFriendList res;
        for (auto& f : friends)
            *res.add_friends() = f;

        session->Send(MakeSendBuffer<Protocol::S_FRIEND_LIST>(res));
    });

    return true;
}

bool ClientPacketHandler::OnHandle_C_START_MINIGAME(GameSessionRef session, const Protocol::CStartMinigame& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (!player) return false;

    // 자기 HomeRoom에서만 시작 가능 (방 주인 권한)
    HomeRoomRef room = GHomeRoomManager->Find(player->GetPlayerId());
    if (!room) return false;

    room->StartMinigame(pkt.game_type(), session);
    return true;
}

bool ClientPacketHandler::OnHandle_C_MINIGAME_RESULT(GameSessionRef session, const Protocol::CMinigameResult& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (!player) return false;

    // 현재 속한 방에 결과 제출
    HomeRoomRef room = session->GetCurrentRoom();
    if (!room) return false;

    room->SubmitResult(player->GetPlayerId(), pkt);
    return true;
}