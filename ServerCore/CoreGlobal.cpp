#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "IocpCore.h"

std::unique_ptr<ThreadManager>		CoreGlobal::_thread = nullptr;
std::unique_ptr<MemoryManager>		CoreGlobal::_memory = nullptr;
std::unique_ptr<IocpCore>			CoreGlobal::_iocpCore = nullptr;
std::unique_ptr<SendBufferManager>	CoreGlobal::_sendBufferManager = nullptr;
std::unique_ptr<JobTimer>			CoreGlobal::_jobTimer = nullptr;
std::unique_ptr<GlobalQueue>		CoreGlobal::_globalQueue = nullptr;
ThreadManager*		GThread = nullptr;
MemoryManager*		GMemory = nullptr;
IocpCore*			GIocpCore = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
JobTimer*			GJobTimer = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;

void CoreGlobal::Init()
{
	_memory = std::make_unique<MemoryManager>();
	GMemory = _memory.get();

	SocketUtils::Init();

	_iocpCore = std::make_unique<IocpCore>();
	GIocpCore = _iocpCore.get();

	_thread = std::make_unique<ThreadManager>();
	GThread = _thread.get();

	_sendBufferManager = std::make_unique<SendBufferManager>();
	GSendBufferManager = _sendBufferManager.get();

	_globalQueue = std::make_unique<GlobalQueue>();
	GGlobalQueue = _globalQueue.get();

	_jobTimer = std::make_unique<JobTimer>();
	GJobTimer = _jobTimer.get();
}

void CoreGlobal::Clear()
{
	GGlobalQueue->Shutdown();
	GGlobalQueue = nullptr;
	_globalQueue = nullptr;

	GJobTimer = nullptr;
	_jobTimer = nullptr;

	GSendBufferManager = nullptr;
	_sendBufferManager = nullptr;

	GThread = nullptr;
	_thread = nullptr;

	GIocpCore = nullptr;
	_iocpCore = nullptr;

	SocketUtils::Clear();

	GMemory = nullptr;
	_memory = nullptr;
}
