#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <spdlog/spdlog.h>
#include "Common/Logging.hpp"

namespace Grindstone {
	class Logger {
	public:
		static void Initialize(std::string path);

		template<typename... Args>
		static void Print(const char* fmt, const Args &... args) {
			logger->info(fmt, args...);
		}

		template<typename... Args>
		static void PrintTrace(const char* fmt, const Args &... args) {
			logger->trace(fmt, args...);
		}

		template<typename... Args>
		static void PrintWarning(const char* fmt, const Args &... args) {
			logger->warn(fmt, args...);
		}

		template<typename... Args>
		static void PrintError(const char* fmt, const Args &... args) {
			logger->error(fmt, args...);
		}

		template<typename... Args>
		static void Print(LogSeverity logSeverity, const char* fmt, const Args &... args) {
			switch (logSeverity) {
			case LogSeverity::Info:
				logger->info(fmt, args...);
				break;
			case LogSeverity::Trace:
				logger->trace(fmt, args...);
				break;
			case LogSeverity::Warning:
				logger->warn(fmt, args...);
				break;
			case LogSeverity::Error:
				logger->error(fmt, args...);
				break;
			}
		}
		~Logger();

	private:
		static spdlog::logger *logger;
	};
}

#endif
