#include "pch.h"
#include "GameGlobal.h"
#include "GameRoom.h"

std::shared_ptr<GameRoom> GameGlobal::_gameRoom = nullptr;
GameRoom* GGameRoom = nullptr;

void GameGlobal::Init()
{
    CoreGlobal::Init();
    Logger::Get().Init("server.log");
    LOG_INFO("Server starting...");

    _gameRoom = MakeShared<GameRoom>();
    GGameRoom = _gameRoom.get();
    GGameRoom->FlushBatch();

    LOG_INFO("GameRoom initialized");
}

void GameGlobal::Clear()
{
    LOG_INFO("Server shutting down...");

    GGameRoom = nullptr;
    _gameRoom = nullptr;

    CoreGlobal::Clear();
}