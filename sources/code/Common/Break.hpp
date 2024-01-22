#pragma once

#if defined(_WIN32)
	#define GS_DEBUG_BREAK __debugbreak()
#elif defined(GS_COMPILER_CLANG)
	#define GS_DEBUG_BREAK __builtin_debugtrap()
#elif defined(GS_COMPILER_ARM)
	#define GS_DEBUG_BREAK __breakpoint(42)
#elif defined(GS_PLATFORM_POSIX)
	#include <signal.h>
	#if defined(SIGTRAP)
		#define GS_DEBUG_BREAK raise(SIGTRAP)
	#else
		#define GS_DEBUG_BREAK raise(SIGABRT)
	#endif
#else
	#define GS_DEBUG_BREAK
#endif
