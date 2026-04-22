#pragma once



/*------------
	Crash
------------*/

#define CRASH(cause)				\
do {									\
	std::cout << cause << std::endl;\
	__debugbreak();					\
	__analysis_assume(false);		\
} while(0)

#define ASSERT_CRASH(expr)		\
do {								\
	if(!(expr))					\
	{							\
		CRASH("ASSERT_CRASH");	\
		__analysis_assume(expr);\
	}							\
} while(0)