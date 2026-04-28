#pragma once
#include "GameSession.h"

class GameRoom
{
public:
    void Enter(GameSessionRef session);
    void Leave(GameSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);

private:
    std::shared_mutex   _rwLock;
    Set<GameSessionRef> _sessions;
};

extern GameRoomRef GGameRoom;