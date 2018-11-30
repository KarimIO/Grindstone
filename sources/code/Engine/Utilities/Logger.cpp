#include "Logger.hpp"

#include <stdarg.h>
#include <iostream>

using namespace Logging;

Logger &Logger::getInstance() {
	static Logger logger;
	return logger;
}

void Logger::setPath(std::string log_path) {
	log_path_ = log_path;
	output_ = fopen(log_path.c_str(), "w");
	if(output_ == NULL) {
		throw std::runtime_error("Could not open log file.\n");
	}
}

void Logger::print(PrintType type, std::string str, ...) {
	std::lock_guard<std::mutex> lock(write_mutex_);

	va_list args;
	va_start(args, str);

	// printToFile(type, str, args);
	printToConsole(type, str, args);

	va_end(args);
}

void Logger::printToFile(PrintType type, std::string str, ...) {
	va_list args;
	va_start(args, str);
	switch (type) {
	default:
	case PRINT_NOTIFICATION:
		vfprintf(output_, (str + "\n").c_str(), args);
		break;
	case PRINT_WARNING:
		vfprintf(output_, ("<WARN>: " + str + "\n").c_str(), args);
		break;
	case PRINT_ERROR:
		vfprintf(output_, ("<ERROR>: " + str + "\n").c_str(), args);
		break;
	case PRINT_FATAL_ERROR:
		vfprintf(output_, ("<FATAL>: " + str + "\n").c_str(), args);
		break;
	}
	va_end(args);
}

void Logger::printToConsole(PrintType type, std::string str, ...) {
	va_list args;
	va_start(args, str);
	switch (type) {
#ifndef _WIN32
	default:
	case PRINT_NOTIFICATION:
		vprintf((str + "\n").c_str(), args);
		break;
	case PRINT_WARNING:
		vprintf(("\033[1;33m" + str + "\033[0m\n").c_str(), args);
		break;
	case PRINT_ERROR:
		vprintf(("\033[1;31m" + str + "\033[0m\n").c_str(), args);
		break;
	case PRINT_FATAL_ERROR:
		vprintf(("\033[1;35m" + str + "\033[0m\n").c_str(), args);
		break;
#else
	default:
	case PRINT_NOTIFICATION:
		vprintf((str + "\n").c_str(), args);
		break;
	case PRINT_WARNING:
		vprintf(("Warning: " + str + "\n").c_str(), args);
		break;
	case PRINT_ERROR:
		vprintf(("Error: " + str + "\n").c_str(), args);
		break;
	case PRINT_FATAL_ERROR:
		vprintf(("Fatal Error: " + str + "\n").c_str(), args);
		break;
#endif
	}
	va_end(args);
}

Logger::~Logger() {
	fclose(output_);
}