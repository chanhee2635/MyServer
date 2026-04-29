// AUTO-GENERATED -- DO NOT EDIT MANUALLY
// Source: Game.proto  /  Run Gen.bat to regenerate
#include "pch.h"
#include "ClientPacketHandler.h"

bool ClientPacketHandler::Handle(GameSessionRef session, std::span<const BYTE> packet, uint16 type)
{
    switch (type)
    {
    case Protocol::C_ENTER: return HandlePacket<Protocol::CEnter, OnHandle_C_ENTER>(session, packet);
    case Protocol::C_COMPLETE_QUEST: return HandlePacket<Protocol::CCompleteQuest, OnHandle_C_COMPLETE_QUEST>(session, packet);
    case Protocol::C_ROOM_CHAT: return HandlePacket<Protocol::CRoomChat, OnHandle_C_ROOM_CHAT>(session, packet);
    case Protocol::C_WHISPER: return HandlePacket<Protocol::CWhisper, OnHandle_C_WHISPER>(session, packet);
    case Protocol::C_INVITE_FRIEND: return HandlePacket<Protocol::CInviteFriend, OnHandle_C_INVITE_FRIEND>(session, packet);
    case Protocol::C_INVITE_RESPONSE: return HandlePacket<Protocol::CInviteResponse, OnHandle_C_INVITE_RESPONSE>(session, packet);
    case Protocol::C_VISIT_REQUEST: return HandlePacket<Protocol::CVisitRequest, OnHandle_C_VISIT_REQUEST>(session, packet);
    case Protocol::C_VISIT_RESPONSE: return HandlePacket<Protocol::CVisitResponse, OnHandle_C_VISIT_RESPONSE>(session, packet);
    case Protocol::C_KICK_PLAYER: return HandlePacket<Protocol::CKickPlayer, OnHandle_C_KICK_PLAYER>(session, packet);
    case Protocol::C_GET_FRIEND_LIST: return HandlePacket<Protocol::CGetFriendList, OnHandle_C_GET_FRIEND_LIST>(session, packet);
    case Protocol::C_START_MINIGAME: return HandlePacket<Protocol::CStartMinigame, OnHandle_C_START_MINIGAME>(session, packet);
    case Protocol::C_MINIGAME_RESULT: return HandlePacket<Protocol::CMinigameResult, OnHandle_C_MINIGAME_RESULT>(session, packet);
    default: 
        LOG_WARN("Unknown packet type=" + std::to_string(type));
        return false;
    }
}
