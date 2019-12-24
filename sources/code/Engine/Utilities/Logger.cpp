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
#include "spdlog/sinks/base_sink.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/common.h"
#include "Core/Engine.hpp"
#include "Core/Editor.hpp"

spdlog::logger *Logger::debug_logger_;


class MySink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{

		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		fmt::memory_buffer formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);
		auto editor = engine.getEditor();
		if (editor)
			editor->printConsoleEntry(fmt::to_string(formatted).c_str());
	}

	void flush_() override
	{
		//std::cout << std::flush;
	}
};

void Logger::init(std::string path) {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::warn);
	//console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	file_sink->set_level(spdlog::level::trace);

	auto my_sink = std::make_shared<MySink>();
	my_sink->set_level(spdlog::level::info);

	debug_logger_ = new spdlog::logger("Debug Logger", { console_sink, file_sink, my_sink });
	debug_logger_->set_level(spdlog::level::debug);
}

spdlog::logger *Logger::getDebugLogger() {
	return debug_logger_;
}
