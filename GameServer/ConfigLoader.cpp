#include "pch.h"
#include "ConfigLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ConfigLoader::Load(const std::string& path, AppConfig& outConfig)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("Config file not found: " + path);
        return false;
    }

    json j;
    try
    {
        file >> j;
    }
    catch (const json::parse_error& e)
    {
        LOG_ERROR("Config parse error: " + std::string(e.what()));
        return false;
    }

    // Database
    if (j.contains("database"))
    {
        auto& db = j["database"];
        if (db.contains("host"))     outConfig.db.host = db["host"].get<std::string>();
        if (db.contains("port"))     outConfig.db.port = db["port"].get<uint32>();
        if (db.contains("user"))     outConfig.db.user = db["user"].get<std::string>();
        if (db.contains("password")) outConfig.db.password = db["password"].get<std::string>();
        if (db.contains("name"))     outConfig.db.name = db["name"].get<std::string>();
        if (db.contains("threads"))  outConfig.db.threads = db["threads"].get<int32>();
    }

    // Server
    if (j.contains("server"))
    {
        auto& sv = j["server"];
        if (sv.contains("port"))        outConfig.server.port = sv["port"].get<uint32>();
        if (sv.contains("maxSessions")) outConfig.server.maxSessions = sv["maxSessions"].get<int32>();
    }

    LOG_INFO("Config loaded: " + path);
    return true;
}