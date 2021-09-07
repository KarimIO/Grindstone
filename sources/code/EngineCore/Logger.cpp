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

class EditorSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		// fmt::memory_buffer formatted;
		// base_sink<std::mutex>::formatter_->format(msg, formatted);
		/*auto editor = engine.getEditor();
		if (editor)
			editor->printConsoleEntry(fmt::to_string(formatted).c_str());*/
	}

	void flush_() override {
		//std::cout << std::flush;
	}
};

void Logger::init(std::string path) {
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::trace);
	//consoleSink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	fileSink->set_level(spdlog::level::trace);

	auto editorSink = std::make_shared<EditorSink>();
	editorSink->set_level(spdlog::level::trace);

	debugLogger = new spdlog::logger("Debug Logger", { consoleSink, fileSink, editorSink });
	debugLogger->set_level(spdlog::level::trace);
	debugLogger->flush_on(spdlog::level::trace);
}

Logger::~Logger() {
	debugLogger->flush();
}

spdlog::logger *Logger::get() {
	return debugLogger;
}
