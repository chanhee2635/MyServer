#include "pch.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "GameRoom.h"
#include "GameDB.h"
#include "DBManager.h"
#include "PlayerManager.h"
#include "HomeRoom.h"
#include "HomeRoomManager.h"
#include "GameGlobal.h"

void GameSession::OnConnected()
{
}

void GameSession::OnDisconnected()
{
    PlayerRef player = GetPlayer();
    if (player)
    {
        uint64 playerId = player->GetPlayerId();

        // 방문 중이었으면 방문한 방에서 퇴장
        HomeRoomRef currentRoom = GetCurrentRoom();
        HomeRoomRef ownRoom     = GHomeRoomManager->Find(playerId);
        if (currentRoom && currentRoom != ownRoom)
            currentRoom->LeaveVisitor(
                std::static_pointer_cast<GameSession>(shared_from_this()));

        // 자기 HomeRoom 닫기 (방문자 전원 퇴장)
        if (ownRoom)
        {
            ownRoom->OwnerLeft();
            GHomeRoomManager->Remove(playerId);
        }

        // 전역 매핑 해제
        GPlayerManager->Unregister(playerId);
        GGameRoom->Leave(std::static_pointer_cast<GameSession>(shared_from_this()));

        // 플레이어 데이터 비동기 저장
        GDBManager->Async([player](DBConnection& conn)
        {
            GameDB::SavePlayer(conn, player);
        });
    }
}

void GameSession::OnRecvPacket(std::span<const BYTE> packet, uint16 type)
{
    ClientPacketHandler::Handle(
        std::static_pointer_cast<GameSession>(shared_from_this()),
        packet, type);
}
