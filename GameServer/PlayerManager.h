#pragma once
#include "GameTypes.h"

class PlayerManager
{
public:
    void Register(uint64 playerId, GameSessionRef session);
    void Unregister(uint64 playerId);

    GameSessionRef Find(uint64 playerId) const;
    int32 Count() const;

private:
    mutable std::shared_mutex           _rwLock;
    HashMap<uint64, GameSessionWeakRef> _sessions;
};
