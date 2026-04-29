#pragma once
#include "GameTypes.h"
#include "JobQueue.h"
#include "Protocol/Game.pb.h"

// ─────────────────────────────────────────────────────────────────────────────
// HomeRoom  :  플레이어 1명의 집 방
//
// - 방 주인(owner)은 항상 존재, 방문자(visitors)는 초대/방문으로 입장
// - JobQueue 상속 → 모든 상태 변경은 Push() 람다 안에서만 수행 (락 불필요)
// - FlushBatch()로 100ms마다 대기 중인 패킷을 일괄 전송
// ─────────────────────────────────────────────────────────────────────────────
class HomeRoom : public JobQueue
{
public:
    explicit HomeRoom(GameSessionRef owner);

    // ── 입장 / 퇴장 ──────────────────────────────────────────────────────────
    void EnterVisitor(GameSessionRef visitor);
    void LeaveVisitor(GameSessionRef visitor);

    // 주인 접속 해제 시: 모든 방문자 강제 퇴장 후 방 정리
    void OwnerLeft();

    // ── 채팅 / 브로드캐스트 ──────────────────────────────────────────────────
    void Broadcast(SendBufferRef sendBuffer);        // 주인 + 방문자 전체
    void BroadcastExcept(SendBufferRef sendBuffer,   // 특정 세션 제외
        GameSessionRef exclude);

    // ── 강퇴 ─────────────────────────────────────────────────────────────────
    void Kick(uint64 targetPlayerId);

    // ── 미니게임 ──────────────────────────────────────────────────────────────
    void StartMinigame(int32 gameType, GameSessionRef requester);
    void SubmitResult (uint64 playerId, const Protocol::CMinigameResult& result);
    bool IsMinigameActive() const { return _minigame != nullptr; }

    // ── 배치 전송 (JobTimer로 100ms마다 호출) ────────────────────────────────
    void FlushBatch();

    // ── 조회 ─────────────────────────────────────────────────────────────────
    GameSessionRef GetOwner() const { return _owner; }
    uint64         GetOwnerId() const;
    int32          GetVisitorCount() const;

private:
    // ── 미니게임 세션 ─────────────────────────────────────────────────────────
    struct MinigameSession
    {
        int32                  gameType = -1;
        int32                  seed     = 0;
        HashMap<uint64, int32> scores;    // playerId → score (-1 = 미제출)
        int32                  expected = 0;
        int32                  received = 0;
    };

    GameSessionRef                   _owner;
    HashSet<GameSessionRef>          _visitors;
    Vector<SendBufferRef>            _pendingBroadcasts;
    std::unique_ptr<MinigameSession> _minigame;
    HashMap<uint64, int32>           _sessionWins;   // 이번 세션 승리 횟수 (접속 중 유지)
};
