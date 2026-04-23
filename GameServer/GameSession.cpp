#include "pch.h"
#include "GameSession.h"

void GameSession::OnConnected()
{
}

void GameSession::OnDisconnected()
{
}

void GameSession::OnRecvPacket(std::span<const BYTE> packet, uint16 type)
{
    SendBufferRef sendBuffer = GSendBufferManager->Open(static_cast<uint32>(packet.size()));
    std::ranges::copy(packet, sendBuffer->GetBuffer());
    sendBuffer->Close(static_cast<uint32>(packet.size()));
    Send(sendBuffer);
}
