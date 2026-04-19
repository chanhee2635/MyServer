#include "pch.h"
#include "CoreTLS.h"

thread_local ThreadType		 LThreadType	 = ThreadType::NONE;
thread_local uint32			 LThreadId		 = 0;
thread_local FrameAllocator* LFrameAllocator = nullptr;

void CoreTLS::OnThreadStart(ThreadType type, uint32 id)
{
	LThreadType = type;
	LThreadId = id;
	LFrameAllocator = new FrameAllocator();
}

void CoreTLS::OnThreadEnd()
{
	if (LFrameAllocator)
	{
		delete LFrameAllocator;
		LFrameAllocator = nullptr;
	}
}

