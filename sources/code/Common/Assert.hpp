#pragma once

#include <Windows.h>
#include <iostream>

#include <Common/Break.hpp>
#include <Common/String.hpp>

#ifdef _DEBUG
	#define GS_ENABLE_ASSERTS
#endif

#ifdef GS_ENABLE_ASSERTS
	#define GS_ASSERT_LOG(text, ...) \
		std::wcout << TEXT(__FILE__) << TEXT("(" << __LINE__ << "): ") << TEXT("Assertion Failed") << std::endl; \
		MessageBox( \
			NULL, \
			TEXT(#text), \
			TEXT("Assertion Failed"), \
			MB_ICONEXCLAMATION | MB_OK \
		);

	#define GS_BREAK_WITH_MESSAGE(msg, ...) \
		GS_ASSERT_LOG(msg, __VA_ARGS__); \
		GS_DEBUG_BREAK;

	#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...) \
		if(!(condition)) { \
			GS_ASSERT_LOG(msg, __VA_ARGS__); \
			GS_DEBUG_BREAK; \
		}

	#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...) \
		if(!(condition)) { \
			GS_ASSERT_LOG(msg, __VA_ARGS__); \
			GS_DEBUG_BREAK; \
		}
	#define GS_ASSERT_ENGINE(condition) \
		if(!(condition)) { \
			GS_ASSERT_LOG(condition); \
			GS_DEBUG_BREAK; \
		}

	#define GS_ASSERT(condition) \
		if(!(condition)) { \
			GS_ASSERT_LOG(condition); \
			GS_DEBUG_BREAK; \
		}
#else
	#define GS_ASSERT_LOG(text, ...)
	#define GS_BREAK_WITH_MESSAGE(msg, ...)
	#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...)
	#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...)
	#define GS_ASSERT_ENGINE(condition)
	#define GS_ASSERT(condition)
#endif
