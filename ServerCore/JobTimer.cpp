#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

void JobTimer::Reserve(uint32 afterMs, JobQueueRef owner, Job job)
{
    TimerItem item;
    item.executeTick = Now() + afterMs;
    item.owner = owner; 
    item.job = std::move(job);

    std::lock_guard lock(_lock);
    _items.push(std::move(item));
}

void JobTimer::Distribute(uint64 nowTick)
{
    while (true)
    {
        TimerItem item;
        {
            std::lock_guard lock(_lock);
            if (_items.empty() || _items.top().executeTick > nowTick)
                break;

            item = std::move(const_cast<TimerItem&>(_items.top()));
            _items.pop();
        }
        if (JobQueueRef queue = item.owner.lock())
        {
            queue->Push(std::move(item.job));
            ServerStats::Get().job.timerFired.fetch_add(1, std::memory_order_relaxed);
        }
    }
}