#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

std::unique_ptr<ThreadManager> CoreGlobal::_threadManager = nullptr;
ThreadManager* GThreadManager = nullptr;

void CoreGlobal::Init()
{
	SocketUtils::Init();

	_threadManager = std::make_unique<ThreadManager>();
	GThreadManager = _threadManager.get();
}

void CoreGlobal::Clear()
{
	GThreadManager = nullptr;
	_threadManager = nullptr;

	SocketUtils::Clear();
}
