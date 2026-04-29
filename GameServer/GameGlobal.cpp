#include "pch.h"
#include "GameGlobal.h"
#include "GameRoom.h"
#include "PlayerManager.h"
#include "HomeRoomManager.h"

std::shared_ptr<GameRoom>         GameGlobal::_gameRoom        = nullptr;
std::unique_ptr<DBManager>        GameGlobal::_dbManager       = nullptr;
std::unique_ptr<PlayerManager>    GameGlobal::_playerManager   = nullptr;
std::unique_ptr<HomeRoomManager>  GameGlobal::_homeRoomManager = nullptr;
AppConfig                         GameGlobal::_config          = {};

GameRoom*        GGameRoom        = nullptr;
DBManager*       GDBManager       = nullptr;
PlayerManager*   GPlayerManager   = nullptr;
HomeRoomManager* GHomeRoomManager = nullptr;

void GameGlobal::Init()
{
    CoreGlobal::Init();
    ServerStats::Get().Init();          // Stats 패널 영역 예약 (로그보다 먼저)
    Logger::Get().Init("server.log");
    LOG_INFO("Server starting...");

    // Config 로드
    if (!ConfigLoader::Load("server.json", _config))
    {
        LOG_ERROR("Failed to load server.json — using defaults");
    }

    // MySQL 라이브러리 초기화 (main 스레드에서 1회)
    mysql_library_init(0, nullptr, nullptr);

    // DB 매니저
    _dbManager = std::make_unique<DBManager>();
    _dbManager->Init(
        _config.db.host.c_str(),
        _config.db.user.c_str(),
        _config.db.password.c_str(),
        _config.db.name.c_str(),
        _config.db.threads,
        _config.db.port);
    GDBManager = _dbManager.get();
    LOG_INFO("DBManager initialized (" + std::to_string(_config.db.threads) + " threads)");

    // 플레이어 매니저
    _playerManager = std::make_unique<PlayerManager>();
    GPlayerManager = _playerManager.get();

    // 홈룸 매니저
    _homeRoomManager = std::make_unique<HomeRoomManager>();
    GHomeRoomManager = _homeRoomManager.get();

    // 게임 룸 (글로벌 브로드캐스트용으로 유지)
    _gameRoom = MakeShared<GameRoom>();
    GGameRoom = _gameRoom.get();
    GGameRoom->FlushBatch();

    LOG_INFO("GameRoom initialized");
}

void GameGlobal::Clear()
{
    LOG_INFO("Server shutting down...");

    GGameRoom      = nullptr;
    _gameRoom      = nullptr;

    GPlayerManager   = nullptr;
    _playerManager   = nullptr;

    GHomeRoomManager = nullptr;
    _homeRoomManager = nullptr;

    if (GDBManager)
    {
        GDBManager->Shutdown();
        GDBManager  = nullptr;
        _dbManager  = nullptr;
    }

    mysql_library_end();

    CoreGlobal::Clear();
}