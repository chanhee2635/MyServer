#include "pch.h"
#include "CoreGlobal.h"

void CoreGlobal::Init()
{
	SocketUtils::Init();

	//_threadManager = std::make_unique<ThreadManager>();
	//GThreadManager = _threadManager.get();
	//_threadManager->Init();
}

void CoreGlobal::Clear()
{
	//if(ThreadManager) GThreadManager->Clear();
	//if(GMemory) GMemory->Clear();
	//GThreadManager = nullptr;
	//GMemory = nullptr;
	//_threadManger = nullptr;

	SocketUtils::Clear();
}
