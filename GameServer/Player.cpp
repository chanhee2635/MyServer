#include "pch.h"
#include "Player.h"

void Player::SetInfo(uint64 playerId, std::string_view username)
{
    _playerId = playerId;
    _username = username;
}