#include "Logger.hpp"

#include <iostream>
#include <filesystem>
#include <stdarg.h>
namespace fs = std::filesystem;

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/common.h"

#include "Common/Event/PrintMessageEvent.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore.hpp"
using namespace Grindstone;

spdlog::logger *Logger::logger;

class EditorSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		fmt::memory_buffer formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);

		std::string str = fmt::to_string(formatted);
		ConsoleMessage consoleMsg = { str };
		auto ev = new Events::PrintMessageEvent(consoleMsg);
		auto dispatcher = EngineCore::GetInstance().GetEventDispatcher();
		dispatcher->Dispatch(ev);
	}

	void flush_() override {
		//std::cout << std::flush;
	}
};

void Logger::Initialize(std::string path) {
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::trace);
	//consoleSink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	fileSink->set_level(spdlog::level::trace);

	auto editorSink = std::make_shared<EditorSink>();
	editorSink->set_level(spdlog::level::trace);

	logger = new spdlog::logger("Debug Logger", { consoleSink, fileSink, editorSink });
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
}

Logger::~Logger() {
	logger->flush();
}
