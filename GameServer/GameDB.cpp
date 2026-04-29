#include "pch.h"
#include "GameDB.h"
#include "PlayerManager.h"
#include "GameGlobal.h"

// ─────────────────────────────────────────────────────────────────────────────
// thread_local Prepared Statement 정의
// (DBConnection이 스레드 전용이므로 lock 없이 캐싱 가능)
// ─────────────────────────────────────────────────────────────────────────────
thread_local DBStmt GameDB::_stmtQuestReward;
thread_local DBStmt GameDB::_stmtFriendList;
thread_local DBStmt GameDB::_stmtSelectPlayer;
thread_local DBStmt GameDB::_stmtUpdateLogin;
thread_local DBStmt GameDB::_stmtInsertPlayer;
thread_local DBStmt GameDB::_stmtInsertStats;
thread_local DBStmt GameDB::_stmtInsertRoom;
thread_local DBStmt GameDB::_stmtInsertTutorial;
thread_local DBStmt GameDB::_stmtSelectStats;
thread_local DBStmt GameDB::_stmtSavePlayer;
thread_local DBStmt GameDB::_stmtSaveStats;
thread_local DBStmt GameDB::_stmtSelectQuests;
thread_local DBStmt GameDB::_stmtCompleteQuest;

// ─────────────────────────────────────────────────────────────────────────────
// 헬퍼: 최초 호출 시 한 번만 Prepare
// ─────────────────────────────────────────────────────────────────────────────
static bool EnsurePrepared(DBStmt& stmt, MYSQL* mysql, const char* sql)
{
    if (stmt.IsPrepared()) return true;
    return stmt.Prepare(mysql, sql);
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadOrCreate
// ─────────────────────────────────────────────────────────────────────────────
PlayerRef GameDB::LoadOrCreate(DBConnection& conn,
    uint64 playerId, const std::string& username)
{
    MYSQL* mysql = conn.GetHandle();
    PlayerRef player = MakeShared<Player>();

    // ── 1. 플레이어 조회 ──────────────────────────────────────────────────────
    EnsurePrepared(_stmtSelectPlayer, mysql,
        "SELECT player_id, username, level, exp, points "
        "FROM players WHERE player_id = ?");

    uint64_t outId = 0;  char outName[64] = {};
    int32_t  outLevel = 0; int64_t outExp = 0; int32_t outPoints = 0;

    _stmtSelectPlayer.BindInUInt64(0, playerId);
    _stmtSelectPlayer.BindOutUInt64(0, outId);
    _stmtSelectPlayer.BindOutString(1, outName, sizeof(outName));
    _stmtSelectPlayer.BindOutInt32 (2, outLevel);
    _stmtSelectPlayer.BindOutInt64 (3, outExp);
    _stmtSelectPlayer.BindOutInt32 (4, outPoints);

    if (_stmtSelectPlayer.Execute() && _stmtSelectPlayer.Fetch())
    {
        // 기존 플레이어
        player->SetId     (outId);
        player->SetUsername(outName);
        player->SetLevel  (outLevel);
        player->SetExp    (outExp);
        player->SetPoints (outPoints);

        // 로그인 시각 갱신
        EnsurePrepared(_stmtUpdateLogin, mysql,
            "UPDATE players SET last_login = NOW() WHERE player_id = ?");
        _stmtUpdateLogin.BindInUInt64(0, playerId);
        _stmtUpdateLogin.Execute();
    }
    else
    {
        // ── 2. 신규 플레이어 생성 ─────────────────────────────────────────────
        EnsurePrepared(_stmtInsertPlayer, mysql,
            "INSERT INTO players (player_id, username) VALUES (?, ?)");
        _stmtInsertPlayer.BindInUInt64(0, playerId);
        _stmtInsertPlayer.BindInString(1, username);
        _stmtInsertPlayer.Execute();

        EnsurePrepared(_stmtInsertStats, mysql,
            "INSERT INTO player_stats (player_id) VALUES (?)");
        _stmtInsertStats.BindInUInt64(0, playerId);
        _stmtInsertStats.Execute();

        EnsurePrepared(_stmtInsertRoom, mysql,
            "INSERT INTO player_rooms (player_id, room_type, is_unlocked) VALUES (?, ?, 1)");
        for (int32 roomType : {0, 2, 4})
        {
            _stmtInsertRoom.BindInUInt64(0, playerId);
            _stmtInsertRoom.BindInInt32 (1, roomType);
            _stmtInsertRoom.Execute();
        }

        EnsurePrepared(_stmtInsertTutorial, mysql,
            "INSERT INTO player_tutorial (player_id) VALUES (?)");
        _stmtInsertTutorial.BindInUInt64(0, playerId);
        _stmtInsertTutorial.Execute();

        player->SetId      (playerId);
        player->SetUsername(username);
        LOG_INFO("New player: " + username + " (" + std::to_string(playerId) + ")");
    }

    // ── 3. 스탯 로드 ─────────────────────────────────────────────────────────
    EnsurePrepared(_stmtSelectStats, mysql,
        "SELECT hygiene, toxicity, stability, comfort, family_hp "
        "FROM player_stats WHERE player_id = ?");

    int32_t hygiene = 0, toxicity = 100, stability = 0, comfort = 0, familyHp = 50;
    _stmtSelectStats.BindInUInt64(0, playerId);
    _stmtSelectStats.BindOutInt32(0, hygiene);
    _stmtSelectStats.BindOutInt32(1, toxicity);
    _stmtSelectStats.BindOutInt32(2, stability);
    _stmtSelectStats.BindOutInt32(3, comfort);
    _stmtSelectStats.BindOutInt32(4, familyHp);

    if (_stmtSelectStats.Execute() && _stmtSelectStats.Fetch())
    {
        PlayerStats s;
        s.hygiene   = hygiene;
        s.toxicity  = toxicity;
        s.stability = stability;
        s.comfort   = comfort;
        s.family_hp = familyHp;
        player->SetStats(s);
    }

    return player;
}

// ─────────────────────────────────────────────────────────────────────────────
// SavePlayer
// ─────────────────────────────────────────────────────────────────────────────
void GameDB::SavePlayer(DBConnection& conn, PlayerRef player)
{
    MYSQL* mysql = conn.GetHandle();

    EnsurePrepared(_stmtSavePlayer, mysql,
        "UPDATE players SET last_logout = NOW(), level = ?, exp = ?, points = ? "
        "WHERE player_id = ?");

    _stmtSavePlayer.BindInInt32 (0, player->GetLevel());
    _stmtSavePlayer.BindInInt32 (1, static_cast<int32_t>(player->GetExp()));   // exp가 int64면 필요시 LONGLONG 바인딩으로 교체
    _stmtSavePlayer.BindInInt32 (2, player->GetPoints());
    _stmtSavePlayer.BindInUInt64(3, player->GetPlayerId());
    _stmtSavePlayer.Execute();

    EnsurePrepared(_stmtSaveStats, mysql,
        "UPDATE player_stats SET "
        "hygiene = ?, toxicity = ?, stability = ?, comfort = ?, family_hp = ? "
        "WHERE player_id = ?");

    const auto& s = player->GetStats();
    _stmtSaveStats.BindInInt32 (0, s.hygiene);
    _stmtSaveStats.BindInInt32 (1, s.toxicity);
    _stmtSaveStats.BindInInt32 (2, s.stability);
    _stmtSaveStats.BindInInt32 (3, s.comfort);
    _stmtSaveStats.BindInInt32 (4, s.family_hp);
    _stmtSaveStats.BindInUInt64(5, player->GetPlayerId());
    _stmtSaveStats.Execute();
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadDailyQuests
// ─────────────────────────────────────────────────────────────────────────────
Vector<Protocol::QuestInfo>
GameDB::LoadDailyQuests(DBConnection& conn, uint64 playerId)
{
    Vector<Protocol::QuestInfo> quests;

    // 오늘 퀘스트가 없을 때만 INSERT (이미 있으면 IGNORE)
    conn.Execute(
        "INSERT IGNORE INTO daily_quests (player_id, quest_id, quest_date) "
        "SELECT " + std::to_string(playerId) + ", quest_id, CURDATE() "
        "FROM quest_definitions");

    // Prepared Statement로 조회
    EnsurePrepared(_stmtSelectQuests, conn.GetHandle(),
        "SELECT q.quest_id, q.name, q.category, q.room_type, "
        "       d.is_done, q.minigame_type "
        "FROM daily_quests d "
        "JOIN quest_definitions q ON d.quest_id = q.quest_id "
        "WHERE d.player_id = ? AND d.quest_date = CURDATE()");

    int32_t questId = 0, category = 0, roomType = 0, minigameType = -1;
    char    name[128] = {};
    bool    isDone = false;

    _stmtSelectQuests.BindInUInt64(0, playerId);
    _stmtSelectQuests.BindOutInt32 (0, questId);
    _stmtSelectQuests.BindOutString(1, name, sizeof(name));
    _stmtSelectQuests.BindOutInt32 (2, category);
    _stmtSelectQuests.BindOutInt32 (3, roomType);
    _stmtSelectQuests.BindOutBool  (4, isDone);
    _stmtSelectQuests.BindOutInt32 (5, minigameType);

    if (!_stmtSelectQuests.Execute()) return quests;

    while (_stmtSelectQuests.Fetch())
    {
        Protocol::QuestInfo q;
        q.set_quest_id     (questId);
        q.set_name         (name);
        q.set_category     (category);
        q.set_room_type    (roomType);
        q.set_is_done      (isDone);
        q.set_minigame_type(minigameType);
        quests.push_back(std::move(q));
    }

    return quests;
}

// ─────────────────────────────────────────────────────────────────────────────
// CompleteQuest
// ─────────────────────────────────────────────────────────────────────────────
bool GameDB::CompleteQuest(DBConnection& conn, uint64 playerId, int32 questId)
{
    EnsurePrepared(_stmtCompleteQuest, conn.GetHandle(),
        "UPDATE daily_quests SET is_done = 1, done_at = NOW() "
        "WHERE player_id = ? AND quest_id = ? "
        "AND quest_date = CURDATE() AND is_done = 0");

    _stmtCompleteQuest.BindInUInt64(0, playerId);
    _stmtCompleteQuest.BindInInt32 (1, questId);
    _stmtCompleteQuest.Execute();

    return _stmtCompleteQuest.AffectedRows() > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetQuestReward
// ─────────────────────────────────────────────────────────────────────────────
GameDB::QuestReward GameDB::GetQuestReward(DBConnection& conn, int32 questId)
{
    EnsurePrepared(_stmtQuestReward, conn.GetHandle(),
        "SELECT exp_reward, point_reward, stat_type, stat_value "
        "FROM quest_definitions WHERE quest_id = ?");

    int32_t exp = 0, pts = 0, statType = -1, statVal = 0;

    _stmtQuestReward.BindInInt32 (0, questId);
    _stmtQuestReward.BindOutInt32(0, exp);
    _stmtQuestReward.BindOutInt32(1, pts);
    _stmtQuestReward.BindOutInt32(2, statType);
    _stmtQuestReward.BindOutInt32(3, statVal);

    QuestReward reward;
    if (_stmtQuestReward.Execute() && _stmtQuestReward.Fetch())
    {
        reward.expGained = exp;
        reward.ptsGained = pts;
        reward.statType  = statType;
        reward.statValue = statVal;
    }
    return reward;
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadFriendList
// ─────────────────────────────────────────────────────────────────────────────
Vector<Protocol::FriendInfo>
GameDB::LoadFriendList(DBConnection& conn, uint64 playerId)
{
    Vector<Protocol::FriendInfo> friends;

    EnsurePrepared(_stmtFriendList, conn.GetHandle(),
        "SELECT f.friend_id, p.username, f.wins, f.losses "
        "FROM friend_records f "
        "JOIN players p ON p.player_id = f.friend_id "
        "WHERE f.player_id = ?");

    uint64_t friendId = 0;
    char     username[64] = {};
    int32_t  wins = 0, losses = 0;

    _stmtFriendList.BindInUInt64 (0, playerId);
    _stmtFriendList.BindOutUInt64(0, friendId);
    _stmtFriendList.BindOutString(1, username, sizeof(username));
    _stmtFriendList.BindOutInt32 (2, wins);
    _stmtFriendList.BindOutInt32 (3, losses);

    if (!_stmtFriendList.Execute()) return friends;

    while (_stmtFriendList.Fetch())
    {
        Protocol::FriendInfo info;
        info.set_player_id(friendId);
        info.set_username (username);
        info.set_wins     (wins);
        info.set_losses   (losses);
        // is_online은 DB가 아닌 PlayerManager로 실시간 확인
        // (DB 워커 스레드에서 호출 가능 — PlayerManager는 shared_mutex로 보호)
        info.set_is_online(GPlayerManager->Find(friendId) != nullptr);
        friends.push_back(std::move(info));
    }

    return friends;
}
