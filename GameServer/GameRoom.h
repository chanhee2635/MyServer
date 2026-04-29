#pragma once

class GameRoom : public JobQueue
{
public:
    void Enter(GameSessionRef session);
    void Leave(GameSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);
    void FlushBatch();

private:
    HashSet<GameSessionRef> _sessions;
    Vector<SendBufferRef> _pendingBroadcasts;
};