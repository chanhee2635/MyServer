#include "pch.h"
#include "DummySession.h"

void DummySession::OnConnected()
{
    Protocol::CEnter pkt;
    const uint64 id = reinterpret_cast<uint64>(this) & 0xFFFFFFFF;
    pkt.mutable_player()->set_playerid(id);
    pkt.mutable_player()->set_username("Dummy_" + std::to_string(id & 0xFFFF));
    Send(MakeSendBuffer<Protocol::C_ENTER>(pkt));
}

void DummySession::OnDisconnected()
{
    _isEntered = false;
}

void DummySession::OnRecvPacket(std::span<const BYTE> packet, uint16 type)
{
    switch (type)
    {
    case Protocol::S_ENTER:
    {
        Protocol::SEnter pkt;
        pkt.ParseFromArray(packet.data() + sizeof(PacketHeader),
            static_cast<int>(packet.size() - sizeof(PacketHeader)));
        if (pkt.success())
            _isEntered = true;
        break;
    }
    case Protocol::S_CHAT:
        break;
    }
}

void DummySession::SendChat()
{
    if (!_isEntered) return;
    Protocol::CChat pkt;
    pkt.set_content("Chat #" + std::to_string(++_chatSeq));
    Send(MakeSendBuffer<Protocol::C_CHAT>(pkt));
}