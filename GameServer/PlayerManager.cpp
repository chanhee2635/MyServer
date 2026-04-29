#include "pch.h"
#include "PlayerManager.h"

void PlayerManager::Register(uint64 playerId, GameSessionRef session)
{
    WRITE_LOCK;
    _sessions[playerId] = session;
}

void PlayerManager::Unregister(uint64 playerId)
{
    WRITE_LOCK;
    _sessions.erase(playerId);
}

GameSessionRef PlayerManager::Find(uint64 playerId) const
{
    READ_LOCK;
    auto it = _sessions.find(playerId);
    if (it == _sessions.end()) return nullptr;
    return it->second.lock();
}

int32 PlayerManager::Count() const
{
    READ_LOCK;
    return static_cast<int32>(_sessions.size());
}