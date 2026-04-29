#include "pch.h"
#include "GameRoom.h"
#include "GameSession.h"

void GameRoom::Enter(GameSessionRef session)
{
    Push([this, session]() {
        _sessions.insert(session);
    });
}

void GameRoom::Leave(GameSessionRef session)
{
    Push([this, session]() {
        _sessions.erase(session);
    });
}

void GameRoom::Broadcast(SendBufferRef sendBuffer)
{
    Push([this, sendBuffer]() {
        _pendingBroadcasts.push_back(sendBuffer);
    });
}

void GameRoom::FlushBatch()
{
    Push([this]() {
        if (!_pendingBroadcasts.empty())
        {
            for (auto& session : _sessions)
                for (auto& buf : _pendingBroadcasts)
                    session->Send(buf);

            _pendingBroadcasts.clear();
        }

        GJobTimer->Reserve(100, shared_from_this(), [this]() {
            FlushBatch();
        });
    });
}
