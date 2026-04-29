#pragma once
#include "DBManager.h"
#include "ConfigLoader.h"

extern class GameRoom*        GGameRoom;
extern class DBManager*       GDBManager;
extern class PlayerManager*   GPlayerManager;
extern class HomeRoomManager* GHomeRoomManager;

class GameGlobal
{
public:
    static void Init();
    static void Clear();

    static const AppConfig& GetConfig() { return _config; }

private:
    static std::shared_ptr<class GameRoom>         _gameRoom;
    static std::unique_ptr<DBManager>              _dbManager;
    static std::unique_ptr<class PlayerManager>    _playerManager;
    static std::unique_ptr<class HomeRoomManager>  _homeRoomManager;
    static AppConfig                               _config;
};
