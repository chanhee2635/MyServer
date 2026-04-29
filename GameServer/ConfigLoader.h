#pragma once

struct DBConfig
{
    std::string host = "127.0.0.1";
    uint32      port = 3306;
    std::string user = "root";
    std::string password;
    std::string name = "household_rpg";
    int32       threads = 2;
};

struct ServerConfig
{
    uint32 port = 7777;
    int32  maxSessions = 2000;
};

struct AppConfig
{
    DBConfig     db;
    ServerConfig server;
};

class ConfigLoader
{
public:
    static bool Load(const std::string& path, AppConfig& outConfig);
};