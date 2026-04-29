#pragma once

extern class GameRoom* GGameRoom;

class GameGlobal
{
public:
    static void Init();
    static void Clear();

private:
    static std::shared_ptr<class GameRoom> _gameRoom;
};
