#pragma once
#include <shared_mutex>
#include "GameTypes.h"

// ─────────────────────────────────────────────────────────────────────────────
// HomeRoomManager  :  playerId → HomeRoom 매핑
//
// - 로그인 시 Create, 로그아웃 시 Remove
// - Find는 읽기 전용이므로 shared_lock(READ_LOCK) 사용
// ─────────────────────────────────────────────────────────────────────────────
class HomeRoomManager
{
public:
    HomeRoomRef Create(uint64 ownerId, GameSessionRef ownerSession);
    void        Remove(uint64 ownerId);
    HomeRoomRef Find  (uint64 ownerId) const;

private:
    mutable std::shared_mutex          _rwLock;
    HashMap<uint64, HomeRoomRef>       _rooms;
};

extern HomeRoomManager* GHomeRoomManager;
