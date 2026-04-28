#include "pch.h"
#include "GameRoom.h"
#include "GameSession.h"

void GameRoom::Enter(GameSessionRef session)
{
    WRITE_LOCK;
    _sessions.insert(session);
}

void GameRoom::Leave(GameSessionRef session)
{
    WRITE_LOCK;
    _sessions.erase(session);
}

void GameRoom::Broadcast(SendBufferRef sendBuffer)
{
    Vector<GameSessionRef> sessions;
    {
        READ_LOCK;
        sessions.assign(_sessions.begin(), _sessions.end());
    }

    for (auto& session : sessions)
        session->Send(sendBuffer);
}