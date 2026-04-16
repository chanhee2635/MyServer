#pragma once

extern class ThreadManager* GThreadManager;

class CoreGlobal
{
public:
	static void Init();
	static void Clear();

private:
	static std::unique_ptr<class ThreadManager> _threadManager;
};