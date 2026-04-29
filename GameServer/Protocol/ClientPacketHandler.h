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
    static bool OnHandle_C_COMPLETE_QUEST(GameSessionRef session, const Protocol::CCompleteQuest& pkt);
    static bool OnHandle_C_ROOM_CHAT(GameSessionRef session, const Protocol::CRoomChat& pkt);
    static bool OnHandle_C_WHISPER(GameSessionRef session, const Protocol::CWhisper& pkt);
    static bool OnHandle_C_INVITE_FRIEND(GameSessionRef session, const Protocol::CInviteFriend& pkt);
    static bool OnHandle_C_INVITE_RESPONSE(GameSessionRef session, const Protocol::CInviteResponse& pkt);
    static bool OnHandle_C_VISIT_REQUEST(GameSessionRef session, const Protocol::CVisitRequest& pkt);
    static bool OnHandle_C_VISIT_RESPONSE(GameSessionRef session, const Protocol::CVisitResponse& pkt);
    static bool OnHandle_C_KICK_PLAYER(GameSessionRef session, const Protocol::CKickPlayer& pkt);
    static bool OnHandle_C_GET_FRIEND_LIST(GameSessionRef session, const Protocol::CGetFriendList& pkt);
    static bool OnHandle_C_START_MINIGAME(GameSessionRef session, const Protocol::CStartMinigame& pkt);
    static bool OnHandle_C_MINIGAME_RESULT(GameSessionRef session, const Protocol::CMinigameResult& pkt);

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
