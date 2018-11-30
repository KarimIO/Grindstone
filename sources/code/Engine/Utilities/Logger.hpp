#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>

namespace Logging {
	enum PrintType {
		PRINT_NOTIFICATION = 0,
		PRINT_WARNING,
		PRINT_ERROR,
		PRINT_FATAL_ERROR
	};

	class Logger {
	public:
		static Logger &getInstance();
		void setPath(std::string log_path);
		void print(PrintType type, std::string str, ...);
		void printToFile(PrintType type, std::string str, ...);
		void printToConsole(PrintType type, std::string str, ...);
		~Logger();
	private:
		std::FILE *output_;
		std::string log_path_;
		std::mutex write_mutex_;
	};
}

#define LOG_LINED(type, str, ...)	printf((std::string(__FILE__) + " (Line " + std::to_string(__LINE__) + "): " + std::string(str)).c_str(), ##__VA_ARGS__);
//Logging::Logger::getInstance().print(type, std::string(__FILE__) + " (Line " + std::to_string(__LINE__) + "): " + str, ##__VA_ARGS__)

#define LOG(str, ...)		printf(std::string(str).c_str(), ##__VA_ARGS__);
//Logging::Logger::getInstance().print(Logging::PRINT_NOTIFICATION,	str, ##__VA_ARGS__)
#define LOG_LINE(str, ...)	LOG_LINED(Logging::PRINT_NOTIFICATION,	str, ##__VA_ARGS__)
#define LOG_WARN(str, ...)	LOG_LINED(Logging::PRINT_WARNING,		str, ##__VA_ARGS__)
#define LOG_ERROR(str, ...)	LOG_LINED(Logging::PRINT_ERROR,			str, ##__VA_ARGS__)
#define LOG_FATAL(str, ...)	LOG_LINED(Logging::PRINT_FATAL_ERROR,	str, ##__VA_ARGS__)

#endif