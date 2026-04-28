#include "pch.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "GameRoom.h"

void GameSession::OnConnected()
{
}

void GameSession::OnDisconnected()
{
    GGameRoom->Leave(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(std::span<const BYTE> packet, uint16 type)
{
    ClientPacketHandler::Handle(
        std::static_pointer_cast<GameSession>(shared_from_this()),
        packet, type);
}
