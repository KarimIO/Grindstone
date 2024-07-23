#include <fmt/format.h>
#include <filesystem>
#include <cstdarg>
#include <chrono>
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <Common/Assert.hpp>
#include <Common/Event/PrintMessageEvent.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/EngineCore.hpp>

#include "Logger.hpp"
using namespace Grindstone;


#ifdef _MSC_VER
const WORD debugColorOff = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

WORD debugColors[] = {
	FOREGROUND_GREEN,											// Trace: green
	FOREGROUND_GREEN | FOREGROUND_BLUE,							// Info: cyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,	// Warning: intense yellow
	FOREGROUND_RED | FOREGROUND_INTENSITY,						// Error: intense red
	BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY	// Fatal
};
#else
const char* ansiColorLogPrefix[] = {
	"\033[1;32m", // Trace: green
	"\033[1;36m", // Info: cyan
	"\033[1;33m", // Warning: yellow
	"\033[1;31m", // Error: red
	"\033[1;35m"  // Fatal: magenta
};
#endif

static Grindstone::Logger::LoggerState* loggerState = nullptr;

Grindstone::Logger::LoggerState* Grindstone::Logger::GetLoggerState() {
	return loggerState;
}

void Grindstone::Logger::SetLoggerState(Grindstone::Logger::LoggerState* newLoggerState) {
	loggerState = newLoggerState;
}

void Grindstone::Logger::Initialize(std::filesystem::path path, Events::Dispatcher* newDispatcher) {
	std::filesystem::create_directories(path.parent_path());
	loggerState = new LoggerState();
	loggerState->outputStream.open(path);
	loggerState->dispatcher = newDispatcher;
}

void Grindstone::Logger::Print(
	LogSeverity logSeverity,
	LogSource logSource,
	LogInternalType internalType,
	const char* filename,
	uint32_t line,
	const char* str
) {
	if (logSeverity > LogSeverity::Fatal) {
		logSeverity = LogSeverity::Error;
	}

	std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::now();
	time_t coarse = std::chrono::system_clock::to_time_t(timepoint);
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> fine =
		std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint);
	const unsigned long long milliseconds = fine.time_since_epoch().count() % 1000u;

	tm tm;
	localtime_s(&tm, &coarse);

	char timeBuffer[sizeof("23:59:59.999")]{};
	size_t timeOffset = std::strftime(timeBuffer, sizeof timeBuffer - 3, "%T.", &tm);
	std::snprintf(
		timeBuffer + timeOffset,
		4, "%03llu",
		milliseconds
	);

	const char* category = logSourceStrings[static_cast<size_t>(logSource)];
	const char* logSeverityPrefix = logSeverityPrefixes[static_cast<size_t>(logSeverity)];

	char* outputPrefixBuffer = new char[sizeof("[] [] []: ") + strlen(logSeverityPrefix) + strlen(category) + strlen(timeBuffer)];
	std::string outputPrefix = std::string("[") + logSeverityPrefix + "] [" + category + "] [" + timeBuffer + "]: ";
	std::string completeMessage = outputPrefix + str + '\n';

#ifdef _MSC_VER
	DWORD outputHandle = logSeverity == LogSeverity::Error
		? STD_ERROR_HANDLE
		: STD_OUTPUT_HANDLE;

	HANDLE hConsole = ::GetStdHandle(outputHandle);
	::SetConsoleTextAttribute(hConsole, debugColors[static_cast<size_t>(logSeverity)]);
	::WriteConsole(hConsole, outputPrefix.c_str(), static_cast<DWORD>(outputPrefix.size()), nullptr, nullptr);

	::SetConsoleTextAttribute(hConsole, debugColorOff);
	::WriteConsole(hConsole, str, static_cast<DWORD>(strlen(str)), nullptr, nullptr);
	::WriteConsole(hConsole, "\n", 1, nullptr, nullptr);
#else
	std::ostream& consoleStream = (logSeverity == LogSeverity::Error)
		? std::cerr
		: std::cout;

	const char* resetColorLog = "\033[0m";
	consoleStream
		<< ansiColorLogPrefix[static_cast<size_t>(logSeverity)]
		<< outputPrefix.c_str()
		<< resetColorLog
		<< str
		<< '\n';
#endif

	if (loggerState != nullptr) {
		if (loggerState->outputStream.is_open()) {
			loggerState->outputStream << completeMessage;
			loggerState->outputStream.flush();
		}

		if (loggerState->dispatcher) {
			const ConsoleMessage consoleMsg = {
				str,
				filename,
				line,
				timepoint,
				logSource,
				internalType,
				logSeverity
			};

			Events::BaseEvent* printEvent = new Events::PrintMessageEvent(consoleMsg);

			loggerState->dispatcher->Dispatch(printEvent);
		}
	}
	
#ifdef _MSC_VER
	::OutputDebugString(completeMessage.c_str());
#endif

	if (logSeverity == LogSeverity::Fatal) {
		GS_DEBUG_BREAK;
	}
}

void Grindstone::Logger::CloseLogger() {
	if (loggerState && loggerState->outputStream.is_open()) {
		loggerState->outputStream.close();
	}
}
