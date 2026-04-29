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

    // ServerStats가 커서 조작 시 동일한 락으로 보호하도록 공개
    static std::mutex& GetConsoleLock() { return _consoleLock; }

    // Stats 패널 높이 설정 (Init 이후 ServerStats가 호출)
    static void ReserveStatsPanel(int lines);
    static int  GetStatsPanelLines() { return _statsPanelLines; }

private:
    Logger() = default;

    std::string LevelToString(LogLevel level);
    std::string Timestamp();

    static std::mutex  _consoleLock;   // Stats와 공유
    static int         _statsPanelLines;
    std::ofstream      _file;
};

#define LOG_INFO(msg) Logger::Get().Write(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::Get().Write(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::Get().Write(LogLevel::ERR,  msg)