#pragma once

class GameRoom
{
public:
    void Enter(GameSessionRef session);
    void Leave(GameSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);

private:
    std::shared_mutex   _rwLock;
    HashSet<GameSessionRef> _sessions;
};