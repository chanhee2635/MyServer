#pragma once
#include "Protocol/Game.pb.h"
#include "Session.h"

// GameServer의 ClientPacketHandler.h와 동일한 헬퍼
template<auto PacketType, typename T>
inline SendBufferRef MakeSendBuffer(const T& msg)
{
    const uint32 bodySize = static_cast<uint32>(msg.ByteSizeLong());
    const uint32 totalSize = sizeof(PacketHeader) + bodySize;
    SendBufferRef sendBuffer = GSendBufferManager->Open(totalSize);
    PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetBuffer());
    header->size = static_cast<uint16>(totalSize);
    header->type = static_cast<uint16>(PacketType);
    msg.SerializeToArray(sendBuffer->GetBuffer() + sizeof(PacketHeader), static_cast<int>(bodySize));
    sendBuffer->Close(totalSize);
    return sendBuffer;
}

class DummySession : public PacketSession
{
public:
    void SendChat();
    bool IsEntered() const { return _isEntered; }

protected:
    void OnConnected() override;
    void OnDisconnected() override;
    void OnRecvPacket(std::span<const BYTE> packet, uint16 type) override;

private:
    std::atomic<bool>  _isEntered = false;
    std::atomic<int32> _chatSeq = 0;
};

using DummySessionRef = std::shared_ptr<DummySession>;