#pragma once
#include "CoreTLS.h"

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	template<typename T, typename... Args>
	void Launch(ThreadType type, T&& callback, Args&&... args);
	void Join();

private:
	std::vector<std::thread> _threads;
	std::mutex _lock;
	std::atomic<int32> _threadIdCounter = 0; 
};

template<typename T, typename... Args>
void ThreadManager::Launch(ThreadType type, T&& callback, Args&&... args)
{
    std::lock_guard<std::mutex> guard(_lock);
	int32 nextId = ++_threadIdCounter;

    _threads.push_back(std::thread([type, nextId, cb = std::forward<T>(callback),...args = std::forward<Args>(args)]() mutable
        {
			CoreTLS::OnThreadStart(type, nextId);
            std::invoke(std::move(cb), std::move(args)...);
			CoreTLS::OnThreadEnd();
        })
    );
}