#pragma once

class GlobalQueue
{
public:
    void Push(JobQueueRef queue);
    void Distribute();
    void Shutdown();

private:
    std::mutex               _lock;
    std::condition_variable  _cv;
    std::queue<JobQueueRef>  _queues;
    std::atomic<bool>        _running = false;
};