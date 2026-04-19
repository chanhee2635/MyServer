#pragma once

extern thread_local ThreadType LThreadType;
extern thread_local int LThreadId;
extern thread_local class FrameAllocator* LFrameAllocator;

class CoreTLS
{
public:
	static void OnThreadStart(ThreadType type, int id);
	static void OnThreadEnd();
};

