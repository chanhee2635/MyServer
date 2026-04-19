#pragma once

extern thread_local ThreadType			  LThreadType;
extern thread_local uint32				  LThreadId;
extern thread_local class FrameAllocator* LFrameAllocator;

class CoreTLS
{
public:
	static void OnThreadStart(ThreadType type, uint32 id);
	static void OnThreadEnd();
};

