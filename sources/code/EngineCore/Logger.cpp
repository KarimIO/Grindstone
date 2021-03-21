#include "Logger.hpp"

#include <filesystem>
namespace fs = std::filesystem;

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/base_sink.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/common.h"

spdlog::logger *Logger::debugLogger;

class MySink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		fmt::memory_buffer formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);
		/*auto editor = engine.getEditor();
		if (editor)
			editor->printConsoleEntry(fmt::to_string(formatted).c_str());*/
	}

	void flush_() override {
		//std::cout << std::flush;
	}
};

void Logger::init(std::string path) {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::trace);
	//console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	file_sink->set_level(spdlog::level::trace);

	auto my_sink = std::make_shared<MySink>();
	my_sink->set_level(spdlog::level::trace);

	debugLogger = new spdlog::logger("Debug Logger", { console_sink, file_sink, my_sink });
	debugLogger->set_level(spdlog::level::trace);
}

spdlog::logger *Logger::get() {
	return debugLogger;
}
