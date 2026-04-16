#include "pch.h"
#include "CoreGlobal.h"

void CoreGlobal::Init()
{
	//_threadManager = std::make_unique<ThreadManager>();
	//GThreadManager = _threadManager.get();
	//_threadManager->Init();

	SocketUtils::Init();
}

void CoreGlobal::Clear()
{
	//if(GIocpCore) GIocpCore->Clear();
	//if(ThreadManager) GThreadManager->Clear();
	//if(GMemory) GMemory->Clear();
	//GThreadManager = nullptr;
	//GMemory = nullptr;
	//_threadManger = nullptr;

	SocketUtils::Clear();
}
