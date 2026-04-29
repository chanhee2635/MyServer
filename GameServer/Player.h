#pragma once
#include "Protocol/Game.pb.h"

struct PlayerStats
{
    int32 hygiene   = 0;
    int32 toxicity  = 100;
    int32 stability = 0;
    int32 comfort   = 0;
    int32 family_hp = 50;
};

struct LevelUpResult
{
    bool  leveledUp = false;
    int32 newLevel  = 1;
};

class Player
{
public:
    void SetId      (uint64 id)                { _playerId = id; }
    void SetUsername(const std::string& name)  { _username = name; }
    void SetLevel   (int32 level)              { _level = level; }
    void SetExp     (int64 exp)                { _exp = exp; }
    void SetPoints  (int32 points)             { _points = points; }
    void SetStats   (const PlayerStats& s)     { _stats = s; }

    // 하위 호환
    void SetInfo(uint64 id, std::string_view username) { _playerId = id; _username = username; }

    uint64             GetPlayerId() const { return _playerId; }
    const std::string& GetUsername() const { return _username; }
    int32              GetLevel()    const { return _level; }
    int64              GetExp()      const { return _exp; }
    int32              GetPoints()   const { return _points; }
    const PlayerStats& GetStats()    const { return _stats; }
    PlayerStats&       GetStats()          { return _stats; }

    // ── 보상 적용 ──────────────────────────────────────────────────────────────
    // exp 누적 후 레벨업 여부 반환
    LevelUpResult AddExp(int64 exp);

    void AddPoints(int32 pts)
    {
        _points += pts;
        if (_points < 0) _points = 0;
    }

    // statType: 0=위생 1=독소 2=안정 3=편안 4=가족HP
    void ApplyStat(int32 statType, int32 value);

    // 레벨업에 필요한 exp (레벨 * 100)
    static int64 ExpNeeded(int32 level) { return level * 100LL; }

private:
    uint64      _playerId = 0;
    std::string _username;
    int32       _level  = 1;
    int64       _exp    = 0;
    int32       _points = 0;
    PlayerStats _stats;
};

using PlayerRef = std::shared_ptr<Player>;