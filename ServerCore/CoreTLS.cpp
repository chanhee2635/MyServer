#include "pch.h"
#include "CoreTLS.h"

thread_local ThreadType LThreadType = ThreadType::NONE;
thread_local int32      LThreadId = 0;
thread_local FrameAllocator* LFrameAllocator = nullptr;

void CoreTLS::OnThreadStart(ThreadType type, int id)
{
	LThreadType = type;
	LThreadId = id;
	LFrameAllocator = new FrameAllocator(10 * 1024 * 1024);
}

void CoreTLS::OnThreadEnd()
{
	if (LFrameAllocator)
	{
		delete LFrameAllocator;
		LFrameAllocator = nullptr;
	}
}

