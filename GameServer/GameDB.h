#pragma once
#include "DBConnection.h"
#include "Player.h"
#include "Protocol/Game.pb.h"

// ─────────────────────────────────────────────────────────────────────────────
// GameDB  :  게임 전용 DB 쿼리 모음
//
// DBConnection은 워커 스레드가 독점하므로 PreparedStatement를
// DBConnection별로 lazy-init하여 캐싱한다.
// 각 함수는 자신에게 필요한 stmt를 첫 호출 때 Prepare → 이후 재사용.
// ─────────────────────────────────────────────────────────────────────────────
class GameDB
{
public:
    // 플레이어 로드 (없으면 신규 생성)
    static PlayerRef LoadOrCreate(DBConnection& conn,
        uint64 playerId, const std::string& username);

    // 플레이어 저장 (접속 해제 시)
    static void SavePlayer(DBConnection& conn, PlayerRef player);

    // 일일 퀘스트 로드
    static Vector<Protocol::QuestInfo>
        LoadDailyQuests(DBConnection& conn, uint64 playerId);

    // 퀘스트 완료 처리
    static bool CompleteQuest(DBConnection& conn,
        uint64 playerId, int32 questId);

    // 친구 목록 조회
    static Vector<Protocol::FriendInfo>
        LoadFriendList(DBConnection& conn, uint64 playerId);

    // 퀘스트 보상 조회
    struct QuestReward
    {
        int32 expGained = 0;
        int32 ptsGained = 0;
        int32 statType  = -1;   // -1 = stat 변화 없음
        int32 statValue = 0;
    };
    static QuestReward GetQuestReward(DBConnection& conn, int32 questId);

private:
    // ── Prepared Statement 캐시 (connection 당 thread_local) ─────────────────
    // thread_local이므로 lock 불필요
    static thread_local DBStmt _stmtSelectPlayer;
    static thread_local DBStmt _stmtUpdateLogin;
    static thread_local DBStmt _stmtInsertPlayer;
    static thread_local DBStmt _stmtInsertStats;
    static thread_local DBStmt _stmtInsertRoom;
    static thread_local DBStmt _stmtInsertTutorial;
    static thread_local DBStmt _stmtSelectStats;
    static thread_local DBStmt _stmtSavePlayer;
    static thread_local DBStmt _stmtSaveStats;
    static thread_local DBStmt _stmtSelectQuests;
    static thread_local DBStmt _stmtCompleteQuest;
    static thread_local DBStmt _stmtQuestReward;
    static thread_local DBStmt _stmtFriendList;
};