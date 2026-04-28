#pragma once

enum class LogLevel : uint8 { INFO, WARN, ERR };

class Logger
{
public:
    static Logger& Get()
    {
        static Logger instance;
        return instance;
    }

    void Init(const std::string& filename);
    void Write(LogLevel level, const std::string& msg);

private:
    Logger() = default;

    std::string        LevelToString(LogLevel level);
    std::string        Timestamp();

    std::mutex         _lock;
    std::ofstream      _file;
};

#define LOG_INFO(msg) Logger::Get().Write(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::Get().Write(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::Get().Write(LogLevel::ERR,  msg)