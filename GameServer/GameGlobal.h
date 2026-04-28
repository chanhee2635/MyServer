#pragma once

extern class GameRoom* GGameRoom;

class GameGlobal
{
public:
    static void Init();
    static void Clear();

private:
    static std::unique_ptr<class GameRoom> _gameRoom;
};
