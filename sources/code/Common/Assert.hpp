#pragma once

#include <Windows.h>
#include <iostream>

#include <Common/Break.hpp>
#include <Common/String.hpp>

#define GS_ASSERT_LOG(text, ...) \
	std::wcout << TEXT(__FILE__) << TEXT("(" << __LINE__ << "): ") << TEXT("Assertion Failed") << std::endl; \
	MessageBox( \
		NULL, \
		TEXT(#text), \
		TEXT("Assertion Failed"), \
		MB_ICONEXCLAMATION | MB_OK \
	);

#define GS_ASSERT_ENGINE(condition, ...) \
	if(!(condition)) { \
		GS_ASSERT_LOG(condition,__VA_ARGS__); \
		GS_DEBUG_BREAK; \
	}

#define GS_ASSERT(condition, ...) \
	if(!(condition)) { \
		GS_ASSERT_LOG(condition, __VA_ARGS__); \
		GS_DEBUG_BREAK; \
	}
