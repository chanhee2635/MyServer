#include "pch.h"
#include "CoreTLS.h"

thread_local ThreadType          LThreadType     = ThreadType::NONE;
thread_local uint32              LThreadId       = 0;
thread_local FrameAllocator*     LFrameAllocator = nullptr;
thread_local ThreadLocalMemory*  LThreadMemory   = nullptr;
thread_local SendBufferChunkRef  LSendBufferChunk = nullptr;


void CoreTLS::OnThreadStart(ThreadType type, uint32 id)
{
	LThreadType     = type;
	LThreadId       = id;
	LFrameAllocator = new FrameAllocator();
	LThreadMemory   = new ThreadLocalMemory();
}

void CoreTLS::OnThreadEnd()
{
	LSendBufferChunk = nullptr;

	delete LThreadMemory;
	LThreadMemory = nullptr;

	delete LFrameAllocator;
	LFrameAllocator = nullptr;
}

