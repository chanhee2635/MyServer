#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
    virtual void OnConnected()    override;
    virtual void OnDisconnected() override;
    virtual void OnRecvPacket(std::span<const BYTE> packet, uint16 type) override;
};

using GameSessionRef = std::shared_ptr<GameSession>;