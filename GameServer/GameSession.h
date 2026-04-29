#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
    virtual void OnConnected()    override;
    virtual void OnDisconnected() override;
    virtual void OnRecvPacket(std::span<const BYTE> packet, uint16 type) override;

    void      SetPlayer(PlayerRef player) { _player.store(player); }
    PlayerRef GetPlayer()           const { return _player.load(); }

    // 현재 속한 HomeRoom (방문 중이면 남의 방, 아니면 자기 방)
    void        SetCurrentRoom(HomeRoomRef room) { _currentRoom = room; }
    HomeRoomRef GetCurrentRoom()           const { return _currentRoom; }

private:
    // DB 워커 스레드(SetPlayer)와 IOCP 스레드(GetPlayer)가 동시 접근하므로
    // std::atomic<shared_ptr> 사용 (C++20, deprecated 방식 대체)
    std::atomic<PlayerRef> _player;
    HomeRoomRef            _currentRoom;
};

