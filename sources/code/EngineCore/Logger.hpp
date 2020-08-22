#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <spdlog/spdlog.h>

class Logger {
public:
	static void init(std::string path);
		
	static spdlog::logger *get();

private:
	static spdlog::logger *debug_logger_;
};

// Client log macros
#define GRIND_TRC(...)			Logger::get()->trace(__VA_ARGS__)
#define GRIND_LOG(...)			Logger::get()->info(__VA_ARGS__)
#define GRIND_WARN(...)			Logger::get()->warn(__VA_ARGS__)
#define GRIND_ERROR(...)		Logger::get()->error(__VA_ARGS__)
#define GRIND_FATAL(...)		GRIND_ERROR(__VA_ARGS__)
//Logger::getDebugLogger()->fatal(__VA_ARGS__)
#endif