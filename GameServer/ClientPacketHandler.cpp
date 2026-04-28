#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameRoom.h"
#include "Player.h"

bool ClientPacketHandler::OnHandle_C_ENTER(GameSessionRef session, const Protocol::CEnter& pkt)
{
    PlayerRef player = MakeShared<Player>();
    player->SetInfo(pkt.player().playerid(), pkt.player().username());
    session->SetPlayer(player);

    GGameRoom->Enter(session);

    Protocol::SEnter res;
    res.set_success(true);
    session->Send(MakeSendBuffer<Protocol::S_ENTER>(res));
    return true;
}

bool ClientPacketHandler::OnHandle_C_CHAT(GameSessionRef session, const Protocol::CChat& pkt)
{
    PlayerRef player = session->GetPlayer();
    if (player == nullptr)
    {
        LOG_WARN("C_CHAT received but player is null");
        return false;
    }

    Protocol::SChat res;
    res.set_username(player->GetUsername());
    res.set_content(pkt.content());
    GGameRoom->Broadcast(MakeSendBuffer<Protocol::S_CHAT>(res));
    return true;
}
