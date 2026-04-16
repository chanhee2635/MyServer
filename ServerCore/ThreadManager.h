#pragma once
#include <functional>

enum class ThreadType : uint8 {
	NONE,
	LOGIC, // 게임 컨텐츠 계산
	IO,    // 네트워크 패킷 송/수신 및 가공
	DB     // 데이터베이스 쿼리 처리
};

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
	std::atomic<int> _threadIdCounter = 0; 
};

extern thread_local ThreadType LThreadType;
extern thread_local int LThreadId;

template<typename T, typename... Args>
void ThreadManager::Launch(ThreadType type, T&& callback, Args&&... args)
{
    std::lock_guard<std::mutex> guard(_lock);
	int nextId = ++_threadIdCounter;

    _threads.push_back(std::thread([type, nextId, cb = std::forward<T>(callback),...args = std::forward<Args>(args)]() mutable
        {
			LThreadType = type;
			LThreadId = nextId;
            std::invoke(std::move(cb), std::move(args)...);
        })
    );
}