#include "pch.h"
#include "Player.h"

LevelUpResult Player::AddExp(int64 exp)
{
    _exp += exp;

    LevelUpResult result;
    // 레벨업 반복 체크 (한 번에 여러 레벨 오를 수 있음)
    while (_exp >= ExpNeeded(_level))
    {
        _exp -= ExpNeeded(_level);
        _level++;
        result.leveledUp = true;
    }
    result.newLevel = _level;
    return result;
}

void Player::ApplyStat(int32 statType, int32 value)
{
    auto clamp = [](int32 v) { return std::max(0, std::min(100, v)); };

    switch (statType)
    {
    case 0: _stats.hygiene   = clamp(_stats.hygiene   + value); break;
    case 1: _stats.toxicity  = clamp(_stats.toxicity  + value); break;
    case 2: _stats.stability = clamp(_stats.stability + value); break;
    case 3: _stats.comfort   = clamp(_stats.comfort   + value); break;
    case 4: _stats.family_hp = clamp(_stats.family_hp + value); break;
    default: break;
    }
}

