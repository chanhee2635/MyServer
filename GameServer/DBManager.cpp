#include "pch.h"
#include "DBManager.h"

void DBManager::Init(const char* host, const char* user,
    const char* password, const char* database,
    int32 threadCount, uint32 port)
{
    _running = true;
    for (int32 i = 0; i < threadCount; i++)
        _threads.emplace_back(&DBManager::WorkerThread, this,
            host, user, password, database, port);

    LOG_INFO("DBManager initialized (" + std::to_string(threadCount) + " threads)");
}

void DBManager::Shutdown()
{
    {
        std::lock_guard lock(_lock);
        _running = false;
    }
    _cv.notify_all();
    for (auto& t : _threads)
        if (t.joinable()) t.join();
}

void DBManager::Async(DBTask task)
{
    {
        std::lock_guard lock(_lock);
        _taskQueue.push(std::move(task));
    }
    _cv.notify_one();
}

void DBManager::WorkerThread(std::string host, std::string user,
    std::string password, std::string database,
    uint32 port)
{
    // 스레드마다 독립적인 커넥션
    DBConnection conn;
    if (!conn.Connect(host.c_str(), user.c_str(),
        password.c_str(), database.c_str(), port))
    {
        LOG_ERROR("DBManager: Worker thread connection failed");
        return;
    }

    while (true)
    {
        DBTask task;
        {
            std::unique_lock lock(_lock);
            _cv.wait(lock, [this] {
                return !_taskQueue.empty() || !_running.load();
                });

            if (!_running && _taskQueue.empty())
                break;

            task = std::move(_taskQueue.front());
            _taskQueue.pop();
        }
        task(conn);
    }
}