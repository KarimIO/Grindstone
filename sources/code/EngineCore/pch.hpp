#pragma once

#include <iostream>

#ifdef _WIN32
	#ifdef ENGINE_CORE
		#define ENGINE_CORE_API __declspec(dllexport)
	#else
		#define ENGINE_CORE_API __declspec(dllimport)
	#endif
#else
	#define ENGINE_CORE_API
#endif