#pragma once
class Player
{
public:
    void SetInfo(uint64 playerId, std::string_view username);

    uint64             GetPlayerId() const { return _playerId; }
    const std::string& GetUsername() const { return _username; }

private:
    uint64      _playerId = 0;
    std::string _username;
};

