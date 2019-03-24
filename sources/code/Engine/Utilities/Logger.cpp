#include "Logger.hpp"

#include <filesystem>
#if __cplusplus < 201703L // If the version of C++ is less than 17
// It was still in the experimental:: namespace
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

spdlog::logger *Logger::debug_logger_;

void Logger::init(std::string path) {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::warn);
	//console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	file_sink->set_level(spdlog::level::trace);

	debug_logger_ = new spdlog::logger("Debug Logger", { console_sink, file_sink });
	debug_logger_->set_level(spdlog::level::debug);
}

spdlog::logger *Logger::getDebugLogger() {
	return debug_logger_;
}
