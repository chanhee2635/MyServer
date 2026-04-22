#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "IocpCore.h"
#include "SendBuffer.h"

std::unique_ptr<ThreadManager>		CoreGlobal::_thread = nullptr;
std::unique_ptr<MemoryManager>		CoreGlobal::_memory = nullptr;
std::unique_ptr<IocpCore>			CoreGlobal::_iocpCore = nullptr;
std::unique_ptr<SendBufferManager>	CoreGlobal::_sendBufferManager = nullptr;
ThreadManager*		GThread = nullptr;
MemoryManager*		GMemory = nullptr;
IocpCore*			GIocpCore = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;

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
}

void CoreGlobal::Clear()
{
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
