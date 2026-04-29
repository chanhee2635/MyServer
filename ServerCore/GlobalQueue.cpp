#include "pch.h"

void GlobalQueue::Shutdown()
{
    _running = false;
    _cv.notify_all();
}

void GlobalQueue::Push(JobQueueRef queue)
{
    {
        std::lock_guard lock(_lock);
        _queues.push(std::move(queue));
    }
    _cv.notify_one();
}

void GlobalQueue::Distribute()
{
    _running = true;
    while (_running)
    {
        JobQueueRef queue;
        {
            std::unique_lock lock(_lock);
            _cv.wait(lock, [this] {
                return !_queues.empty() || !_running.load();
            });

            if (!_running && _queues.empty())
                break;

            queue = std::move(_queues.front());
            _queues.pop();
        }
        queue->Execute();
    }
}