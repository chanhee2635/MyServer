#include "pch.h"
#include "JobQueue.h"

void JobQueue::Push(Job job)
{
    ServerStats::Get().job.jobsQueued.fetch_add(1, std::memory_order_relaxed);

    bool enqueue = false;
    {
        std::lock_guard lock(_lock);
        _jobs.push(std::move(job)); 
        if (!_pending)
        {
            _pending = true;
            enqueue = true;
        }
    }

    if (enqueue)
        GGlobalQueue->Push(shared_from_this());
}

void JobQueue::Execute()
{
    LCurrentJobQueue = this;
    const uint64 startTick = ::GetTickCount64();

    while (true)
    {
        Job job;
        {
            std::lock_guard lock(_lock);

            if (_jobs.empty())
            {
                _pending = false; 
                break;
            }

            if (::GetTickCount64() - startTick > Config::Job::MAX_WORK_TICK)
            {
                ServerStats::Get().job.timeSlices.fetch_add(1, std::memory_order_relaxed);

                GGlobalQueue->Push(shared_from_this());
                break;
            }

            job = std::move(_jobs.front()); 
            _jobs.pop();
        }
        job();
        ServerStats::Get().job.jobsExecuted.fetch_add(1, std::memory_order_relaxed);
    }

    LCurrentJobQueue = nullptr;
}