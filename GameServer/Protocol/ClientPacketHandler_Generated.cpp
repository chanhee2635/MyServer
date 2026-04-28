#include "pch.h"
#include "ClientPacketHandler.h"

bool ClientPacketHandler::Handle(GameSessionRef session, std::span<const BYTE> packet, uint16 type)
{
    switch (type)
    {
    case Protocol::C_ENTER: return HandlePacket<Protocol::CEnter, OnHandle_C_ENTER>(session, packet);
    case Protocol::C_CHAT: return HandlePacket<Protocol::CChat, OnHandle_C_CHAT>(session, packet);
    default: 
        LOG_WARN("Unknown packet type=" + std::to_string(type));
        return false;
    }
}
