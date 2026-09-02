#pragma once

#include <Windows.h>
#include <source_location>
#include <format>
#include <string>
#include <iostream>

#include <Common/Break.hpp>
#include <Common/String.hpp>

#ifdef _DEBUG
	#define GS_ENABLE_ASSERTS
#endif

namespace Grindstone::Debug {
	template<typename... Args>
	void AssertLog(
		std::format_string<Args...> format,
		const ::std::source_location& location = ::std::source_location::current(),
		Args&&... args
	) {
		const std::string message = std::format(
			format,
			::std::forward<Args>(args)...
		);

		const std::string fullMessage = std::format(
			"{}({}): Assertion Failed\n"
			"Function: {}\n\n"
			"{}",
			location.file_name(),
			location.line(),
			location.function_name(),
			message
		);

		std::cerr << fullMessage << std::endl;

		const int wideSize = MultiByteToWideChar(
			CP_UTF8,
			0,
			fullMessage.data(),
			static_cast<int>(fullMessage.size()),
			nullptr,
			0
		);

		std::wstring wideMessage(wideSize, L'\0');

		MultiByteToWideChar(
			CP_UTF8,
			0,
			fullMessage.data(),
			static_cast<int>(fullMessage.size()),
			wideMessage.data(),
			wideSize
		);

		MessageBoxW(
			nullptr,
			wideMessage.c_str(),
			L"Assertion Failed",
			MB_ICONEXCLAMATION | MB_OK
		);
	}
}

#ifdef GS_ENABLE_ASSERTS

#define GS_ASSERT_LOG(msg, ...) \
	::Grindstone::Debug::AssertLog(msg, std::source_location::current() __VA_OPT__(,) __VA_ARGS__)

#define GS_BREAK_WITH_MESSAGE(msg, ...) \
	do { \
		GS_ASSERT_LOG(msg __VA_OPT__(,) __VA_ARGS__); \
		GS_DEBUG_BREAK; \
	} while (false)

#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...) \
	do { \
		if (!(condition)) { \
			GS_ASSERT_LOG(msg __VA_OPT__(,) __VA_ARGS__); \
			GS_DEBUG_BREAK; \
		} \
	} while (false)

#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...) \
	do { \
		if (!(condition)) { \
			GS_ASSERT_LOG(msg __VA_OPT__(,) __VA_ARGS__); \
			GS_DEBUG_BREAK; \
		} \
	} while (false)

#define GS_ASSERT_ENGINE(condition) \
	do { \
		if (!(condition)) { \
			GS_ASSERT_LOG("Assertion failed: {}", #condition); \
			GS_DEBUG_BREAK; \
		} \
	} while (false)

#define GS_ASSERT(condition) \
	do { \
		if (!(condition)) { \
			GS_ASSERT_LOG("Assertion failed: {}", #condition); \
			GS_DEBUG_BREAK; \
		} \
	} while (false)

#else

#define GS_ASSERT_LOG(msg, ...)
#define GS_BREAK_WITH_MESSAGE(msg, ...)
#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...)
#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...)
#define GS_ASSERT_ENGINE(condition)
#define GS_ASSERT(condition)

#endif
