#pragma once

#include <iostream>

#ifdef ENGINE_CORE
	#define ENGINE_CORE_API __declspec(dllexport)
#else
	#define ENGINE_CORE_API __declspec(dllimport)
#endif