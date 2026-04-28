#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
    virtual void OnConnected()    override;
    virtual void OnDisconnected() override;
    virtual void OnRecvPacket(std::span<const BYTE> packet, uint16 type) override;

    void SetPlayer(PlayerRef player) { _player = player; }
    PlayerRef GetPlayer() const { return _player; }

private:
    PlayerRef _player;
};

