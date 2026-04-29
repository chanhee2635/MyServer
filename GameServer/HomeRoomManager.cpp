#include "pch.h"
#include "HomeRoomManager.h"
#include "HomeRoom.h"

// GHomeRoomManager 정의는 GameGlobal.cpp 에서

HomeRoomRef HomeRoomManager::Create(uint64 ownerId, GameSessionRef ownerSession)
{
    HomeRoomRef room = MakeShared<HomeRoom>(ownerSession);

    {
        WRITE_LOCK;
        _rooms[ownerId] = room;
    }

    room->FlushBatch();
    return room;
}

void HomeRoomManager::Remove(uint64 ownerId)
{
    WRITE_LOCK;
    _rooms.erase(ownerId);
}

HomeRoomRef HomeRoomManager::Find(uint64 ownerId) const
{
    READ_LOCK;
    auto it = _rooms.find(ownerId);
    return (it != _rooms.end()) ? it->second : nullptr;
}
