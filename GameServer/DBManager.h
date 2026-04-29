#pragma once
#include "DBConnection.h"

using DBTask = std::function<void(DBConnection&)>;

class DBManager
{
public:
    void Init(const char* host, const char* user,
        const char* password, const char* database,
        int32 threadCount = 2, uint32 port = 3306);
    void Shutdown();
    void Async(DBTask task);

private:
    void WorkerThread(std::string host, std::string user,
        std::string password, std::string database, uint32 port);

    std::vector<std::thread> _threads;   // thread는 이동 전용, Pool 할당 의미 없음
    Queue<DBTask>            _taskQueue; // 태스크는 상시 push/pop → Pool 적용
    std::mutex               _lock;
    std::condition_variable  _cv;
    std::atomic<bool>        _running = false;
};