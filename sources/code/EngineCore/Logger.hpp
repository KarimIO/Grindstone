#pragma once

#include <fstream>
#include <fmt/format.h>

#include "Common/Logging.hpp"

namespace Grindstone::Events {
	class Dispatcher;
}

namespace Grindstone::Logger {
	struct LoggerState {
		std::ofstream outputStream;
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
		std::string formattedString = fmt::format(fmt, args...);
		Print(logSeverity, logSource, internalType, filename, line, formattedString.c_str());
	}
}

#define GPRINT_TYPED(severity, source, internalType, str) Grindstone::Logger::Print(severity, source, internalType, __FILE__, __LINE__, str)
#define GPRINT_TYPED_INFO(source, internalType, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, internalType, __FILE__, __LINE__, str)
#define GPRINT_TYPED_TRACE(source, internalType, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, internalType, __FILE__, __LINE__, str)
#define GPRINT_TYPED_WARN(source, internalType, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, internalType, __FILE__, __LINE__, str)
#define GPRINT_TYPED_ERROR(source, internalType, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, internalType, __FILE__, __LINE__, str)
#define GPRINT_TYPED_FATAL(source, internalType, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, internalType, __FILE__, __LINE__, str)

#define GPRINT(severity, source, str) Grindstone::Logger::Print(severity, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)
#define GPRINT_INFO(source, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)
#define GPRINT_TRACE(source, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)
#define GPRINT_WARN(source, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)
#define GPRINT_ERROR(source, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)
#define GPRINT_FATAL(source, str) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, str)

#define GPRINT_TYPED_V(severity, source, internalType, fmt, ...) Grindstone::Logger::Print(severity, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TYPED_INFO_V(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TYPED_TRACE_V(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TYPED_WARN_V(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TYPED_ERROR_V(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TYPED_FATAL_V(source, internalType, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, internalType, __FILE__, __LINE__, fmt, __VA_ARGS__)

#define GPRINT_V(severity, source, fmt, ...) Grindstone::Logger::Print(severity, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_INFO_V(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Info, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_TRACE_V(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Trace, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_WARN_V(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Warning, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_ERROR_V(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Error, source, Grindstone::LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define GPRINT_FATAL_V(source, fmt, ...) Grindstone::Logger::Print(Grindstone::LogSeverity::Fatal, source, LOG_UNSPECIFIED_INTERNAL_TYPE, __FILE__, __LINE__, fmt, __VA_ARGS__)
