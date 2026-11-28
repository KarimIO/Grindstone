#pragma once

#include <format>
#include <functional>
#include <source_location>
#include <string>

#include <Common/Break.hpp>

#ifdef _DEBUG
	#define GS_ENABLE_ASSERTS
#endif

namespace Grindstone::Debug {
	enum class AssertAction { Break, Continue };

	struct AssertInfo {
		std::source_location location;
		std::string message;
	};

	using AssertHandler = std::function<AssertAction(const AssertInfo&)>;

	// Returns the previous handler so callers can restore it.
	AssertHandler SetAssertHandler(AssertHandler handler);
	AssertAction ReportAssert(const AssertInfo& info);
	AssertAction DefaultAssertHandler(const AssertInfo& info);
}


#ifdef GS_ENABLE_ASSERTS

	#define GS_ASSERT_LOG(msg, ...)                                \
		do {                                                          \
			if (::Grindstone::Debug::ReportAssert(                    \
					{std::source_location::current(),                 \
					 std::format(msg __VA_OPT__(, ) __VA_ARGS__)}) == \
				::Grindstone::Debug::AssertAction::Break) {           \
				GS_DEBUG_BREAK;                                       \
			}                                                         \
		} while (false)

	#define GS_BREAK_WITH_MESSAGE(msg, ...) \
		GS_ASSERT_LOG(msg __VA_OPT__(, ) __VA_ARGS__)

	#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...)                         \
		do {                                                                    \
			if (!(condition)) GS_ASSERT_LOG(msg __VA_OPT__(, ) __VA_ARGS__); \
		} while (false)

	#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...) \
		GS_ASSERT_WITH_MESSAGE(condition, msg __VA_OPT__(, ) __VA_ARGS__)

	#define GS_ASSERT(condition)                                      \
		do {                                                          \
			if (!(condition))                                         \
				GS_ASSERT_LOG("Assertion failed: {}", #condition); \
		} while (false)

	#define GS_ASSERT_ENGINE(condition) GS_ASSERT(condition)

#else

#define GS_ASSERT_LOG(msg, ...)
#define GS_BREAK_WITH_MESSAGE(msg, ...)
#define GS_ASSERT_ENGINE_WITH_MESSAGE(condition, msg, ...)
#define GS_ASSERT_WITH_MESSAGE(condition, msg, ...)
#define GS_ASSERT_ENGINE(condition)
#define GS_ASSERT(condition)

#endif
