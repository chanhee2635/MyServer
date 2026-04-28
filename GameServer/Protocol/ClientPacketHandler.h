// AUTO-GENERATED -- DO NOT EDIT MANUALLY
// Source: Game.proto  /  Run Gen.bat to regenerate
#pragma once
#include "Game.pb.h"
#include "GameSession.h"

template<auto PacketType, typename T>
inline SendBufferRef MakeSendBuffer(const T& msg)
{
    const uint32 bodySize  = static_cast<uint32>(msg.ByteSizeLong());
    const uint32 totalSize = sizeof(PacketHeader) + bodySize;

    SendBufferRef sendBuffer = GSendBufferManager->Open(totalSize);

    PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetBuffer());
    header->size = static_cast<uint16>(totalSize);
    header->type = static_cast<uint16>(PacketType);

    msg.SerializeToArray(sendBuffer->GetBuffer() + sizeof(PacketHeader), static_cast<int>(bodySize));
    sendBuffer->Close(totalSize);
    return sendBuffer;
}

class ClientPacketHandler
{
public:
    static bool Handle(GameSessionRef session, std::span<const BYTE> packet, uint16 type);

    static bool OnHandle_C_ENTER(GameSessionRef session, const Protocol::CEnter& pkt);
    static bool OnHandle_C_CHAT(GameSessionRef session, const Protocol::CChat& pkt);

private:
    template<typename MsgType, bool(*OnHandle)(GameSessionRef, const MsgType&)>
    static bool HandlePacket(GameSessionRef session, std::span<const BYTE> packet)
    {
        MsgType pkt;
        if (!pkt.ParseFromArray(
                packet.data() + sizeof(PacketHeader),
                static_cast<int>(packet.size() - sizeof(PacketHeader))))
            return false;
        return OnHandle(session, pkt);
    }
};
