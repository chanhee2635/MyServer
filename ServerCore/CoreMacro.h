#pragma once



/*------------
	Crash
------------*/

#define CRASH(cause)				\
{									\
	std::cout << cause << std::endl;\
	__debugbreak();					\
	__analysis_assume(false);		\
}

#define ASSERT_CRASH(expr)		\
{								\
	if(!(expr))					\
	{							\
		CRASH("ASSERT_CRASH");	\
		__analysis_assume(expr);\
	}							\
}