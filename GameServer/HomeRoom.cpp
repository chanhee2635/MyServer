#include "pch.h"
#include "HomeRoom.h"
#include "GameSession.h"
#include "Player.h"
#include "DBManager.h"
#include "GameDB.h"
#include "GameGlobal.h"
#include "Protocol/ClientPacketHandler.h"

HomeRoom::HomeRoom(GameSessionRef owner)
    : _owner(owner)
{
}

uint64 HomeRoom::GetOwnerId() const
{
    PlayerRef player = _owner->GetPlayer();
    return player ? player->GetPlayerId() : 0;
}

int32 HomeRoom::GetVisitorCount() const
{
    return static_cast<int32>(_visitors.size());
}

// ─────────────────────────────────────────────────────────────────────────────

void HomeRoom::EnterVisitor(GameSessionRef visitor)
{
    Push([this, visitor]()
    {
        _visitors.insert(visitor);

        // 방 안 모든 사람에게 입장 알림
        PlayerRef p = visitor->GetPlayer();
        if (!p) return;

        Protocol::SPlayerEntered notify;
        notify.set_player_id(p->GetPlayerId());
        notify.set_username(p->GetUsername());
        auto buf = MakeSendBuffer<Protocol::S_PLAYER_ENTERED>(notify);

        // 주인에게
        _owner->Send(buf);
        // 기존 방문자들에게
        for (auto& v : _visitors)
            if (v != visitor)
                v->Send(buf);
    });
}

void HomeRoom::LeaveVisitor(GameSessionRef visitor)
{
    Push([this, visitor]()
    {
        if (_visitors.erase(visitor) == 0) return;

        // 남은 사람들에게 퇴장 알림
        PlayerRef p = visitor->GetPlayer();
        if (!p) return;

        Protocol::SPlayerLeft notify;
        notify.set_player_id(p->GetPlayerId());
        notify.set_username(p->GetUsername());
        auto buf = MakeSendBuffer<Protocol::S_PLAYER_LEFT>(notify);

        _owner->Send(buf);
        for (auto& v : _visitors)
            v->Send(buf);
    });
}

void HomeRoom::OwnerLeft()
{
    Push([this]()
    {
        // 모든 방문자에게 S_KICKED 전송
        Protocol::SKicked kicked;
        auto buf = MakeSendBuffer<Protocol::S_KICKED>(kicked);
        for (auto& v : _visitors)
            v->Send(buf);

        _visitors.clear();
        _pendingBroadcasts.clear();
    });
}

void HomeRoom::Kick(uint64 targetPlayerId)
{
    Push([this, targetPlayerId]()
    {
        GameSessionRef target = nullptr;
        for (auto& v : _visitors)
        {
            PlayerRef p = v->GetPlayer();
            if (p && p->GetPlayerId() == targetPlayerId)
            {
                target = v;
                break;
            }
        }
        if (!target) return;

        // 강퇴 대상에게 알림
        Protocol::SKicked kicked;
        target->Send(MakeSendBuffer<Protocol::S_KICKED>(kicked));

        _visitors.erase(target);

        // 나머지에게 퇴장 알림
        PlayerRef p = target->GetPlayer();
        if (!p) return;

        Protocol::SPlayerLeft notify;
        notify.set_player_id(p->GetPlayerId());
        notify.set_username(p->GetUsername());
        auto buf = MakeSendBuffer<Protocol::S_PLAYER_LEFT>(notify);

        _owner->Send(buf);
        for (auto& v : _visitors)
            v->Send(buf);
    });
}

// ─────────────────────────────────────────────────────────────────────────────

void HomeRoom::StartMinigame(int32 gameType, GameSessionRef requester)
{
    Push([this, gameType, requester]()
    {
        if (requester != _owner) return;  // 방 주인만 시작 가능
        if (_minigame)           return;  // 이미 진행 중

        _minigame           = std::make_unique<MinigameSession>();
        _minigame->gameType = gameType;
        _minigame->seed     = static_cast<int32>(
            std::chrono::steady_clock::now().time_since_epoch().count() & 0x7FFFFFFF);

        // 참가자 등록 (주인 + 방문자)
        auto addParticipant = [&](GameSessionRef s)
        {
            PlayerRef p = s->GetPlayer();
            if (p) _minigame->scores[p->GetPlayerId()] = -1;
        };
        addParticipant(_owner);
        for (auto& v : _visitors) addParticipant(v);

        _minigame->expected = static_cast<int32>(_minigame->scores.size());

        // gameType 0 = 바닥청소 (횟수 기반, 시간 제한 없음)
        // gameType 1 = 물건정리 / 2 = 빨래 (60초 제한)
        const int32 duration = (gameType == 0) ? 0 : 60;

        Protocol::SMinigameReady ready;
        ready.set_game_type   (gameType);
        ready.set_seed        (_minigame->seed);
        ready.set_duration_sec(duration);

        auto buf = MakeSendBuffer<Protocol::S_MINIGAME_READY>(ready);
        _owner->Send(buf);
        for (auto& v : _visitors) v->Send(buf);
    });
}

void HomeRoom::SubmitResult(uint64 playerId, const Protocol::CMinigameResult& result)
{
    Push([this, playerId, result]()
    {
        if (!_minigame) return;

        auto it = _minigame->scores.find(playerId);
        if (it == _minigame->scores.end()) return;  // 참가자가 아님
        if (it->second != -1)              return;  // 이미 제출함

        it->second = result.score();
        _minigame->received++;

        if (_minigame->received < _minigame->expected) return;

        // ── 전원 제출 완료 → 순위 계산 ───────────────────────────────────────

        // 점수 내림차순 정렬 (FrameVector: 결과 계산 후 즉시 버려짐)
        Vector<std::pair<uint64, int32>> sorted(
            _minigame->scores.begin(), _minigame->scores.end());
        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        // 순위 → 보상 테이블 (exp, points)
        static constexpr std::array<std::pair<int32, int32>, 4> kRewards = {{
            {50, 30}, {30, 20}, {15, 10}, {5, 5}
        }};

        Protocol::SMinigameRoomResult roomResult;

        // 결과 처리 람다 (주인/방문자 공통)
        auto process = [&](GameSessionRef s)
        {
            PlayerRef p = s->GetPlayer();
            if (!p) return;

            const uint64 pid   = p->GetPlayerId();
            const int32  score = _minigame->scores[pid];

            // 순위 산출 (1-based)
            int32 rank = 1;
            for (auto& [id, sc] : sorted)
            {
                if (id == pid) break;
                rank++;
            }

            const int32 idx = std::min(rank - 1, 3);
            const int32 expR = kRewards[idx].first;
            const int32 ptsR = kRewards[idx].second;

            if (rank == 1) _sessionWins[pid]++;

            // 보상 메모리 적용
            p->AddExp(expR);
            p->AddPoints(ptsR);

            // 개인 보상 패킷
            Protocol::SMinigameReward reward;
            reward.set_exp_gained(expR);
            reward.set_pts_gained(ptsR);
            s->Send(MakeSendBuffer<Protocol::S_MINIGAME_REWARD>(reward));

            // 방 전체 결과에 추가
            auto* entry = roomResult.add_results();
            entry->set_player_id   (pid);
            entry->set_username    (p->GetUsername());
            entry->set_score       (score);
            entry->set_ranking     (rank);
            entry->set_points_reward(ptsR);
            entry->set_session_wins(_sessionWins[pid]);

            // DB 비동기 저장 (보상 반영)
            GDBManager->Async([p](DBConnection& conn)
            {
                GameDB::SavePlayer(conn, p);
            });
        };

        process(_owner);
        for (auto& v : _visitors) process(v);

        // 방 전체에 결과 브로드캐스트
        auto buf = MakeSendBuffer<Protocol::S_MINIGAME_ROOM_RESULT>(roomResult);
        _owner->Send(buf);
        for (auto& v : _visitors) v->Send(buf);

        // 미니게임 세션 종료
        _minigame.reset();
    });
}

// ─────────────────────────────────────────────────────────────────────────────

void HomeRoom::Broadcast(SendBufferRef sendBuffer)
{
    Push([this, sendBuffer]()
    {
        _pendingBroadcasts.push_back(sendBuffer);
    });
}

void HomeRoom::BroadcastExcept(SendBufferRef sendBuffer, GameSessionRef exclude)
{
    Push([this, sendBuffer, exclude]()
    {
        // exclude 세션만 빼고 즉시 전송 (채팅 발신자 제외용)
        if (_owner != exclude) _owner->Send(sendBuffer);
        for (auto& v : _visitors)
            if (v != exclude) v->Send(sendBuffer);
    });
}

void HomeRoom::FlushBatch()
{
    Push([this]()
    {
        if (!_pendingBroadcasts.empty())
        {
            for (auto& buf : _pendingBroadcasts)
            {
                _owner->Send(buf);
                for (auto& v : _visitors)
                    v->Send(buf);
            }
            _pendingBroadcasts.clear();
        }

        GJobTimer->Reserve(100, shared_from_this(), [this]()
        {
            FlushBatch();
        });
    });
}
