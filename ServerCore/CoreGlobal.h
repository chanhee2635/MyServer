#pragma once

extern class ThreadManager*		GThread;
extern class MemoryManager*		GMemory;
extern class IocpCore*			GIocpCore;
extern class SendBufferManager* GSendBufferManager;
extern class JobTimer*			GJobTimer;
extern class GlobalQueue*		GGlobalQueue;

class CoreGlobal
{
public:
	static void Init();
	static void Clear();

private:
	static std::unique_ptr<class ThreadManager>		_thread;
	static std::unique_ptr<class MemoryManager>		_memory;
	static std::unique_ptr<class IocpCore>			_iocpCore;
	static std::unique_ptr<class SendBufferManager> _sendBufferManager;
	static std::unique_ptr<class JobTimer>			_jobTimer;
	static std::unique_ptr<class GlobalQueue>		_globalQueue;
};