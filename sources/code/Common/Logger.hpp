#pragma once

#include <fstream>
#include <format>
#include <mutex>

#include "Common/Logging.hpp"

namespace Grindstone::Events {
	class Dispatcher;
}

namespace Grindstone::Logger {
	struct LoggerState {
		std::ofstream outputStream;
		std::mutex mutex;
		Events::Dispatcher* dispatcher = nullptr;
	};

	LoggerState* GetLoggerState();
	void SetLoggerState(LoggerState*);

	void Initialize(std::filesystem::path path, Grindstone::Events::Dispatcher* newDispatcher);
	void CloseLogger();

	void Print(
		LogSeverity logSeverity,
		LogSource logSource,
		LogInternalType internalType,
		const char* filename,
		uint32_t line,
		const char* str
	);

	template<typename... Args>
	static void Print(
		LogSeverity logSeverity,
		LogSource logSource,
		LogInternalType internalType,
		const char* filename,
		uint32_t line,
		const char* fmt,
		const Args &... args
	) {
		std::string formattedString = std::vformat(fmt, std::make_format_args(args...));
		Print(logSeverity, logSource, internalType, filename, line, formattedString.c_str());
	}
}

#define GPRINT_TYPED(severity, source, internalType, fmt, ...) Grindstone::Logger::Print(severity, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TYPED_INFO(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TYPED_TRACE(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TYPED_WARN(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TYPED_ERROR(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TYPED_FATAL(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, internalType, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define GPRINT(severity, source, fmt, ...) Grindstone::Logger::Print(severity, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_INFO(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_TRACE(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_WARN(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_ERROR(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#define GPRINT_FATAL(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
