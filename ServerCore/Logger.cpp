#include "pch.h"
#include "Logger.h"

void Logger::Init(const std::string& filename)
{
    std::lock_guard<std::mutex> guard(_lock);
    if (_file.is_open()) return;
    _file.open(filename, std::ios::app);
}

void Logger::Write(LogLevel level, const std::string& msg)
{
    std::string line = Timestamp()
        + " [" + LevelToString(level) + "]"
        + " [T" + std::to_string(LThreadId) + "]"
        + " " + msg + "\n";

    std::lock_guard<std::mutex> guard(_lock);
    std::cout << line;
    if (_file.is_open())
        _file << line;
}

std::string Logger::Timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time);

    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

std::string Logger::LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARN: return "WARN";
    case LogLevel::ERR:  return "ERROR";
    }
    return "UNKNOWN";
}