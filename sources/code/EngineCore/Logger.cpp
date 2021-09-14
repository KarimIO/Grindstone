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
		fmt::memory_buffer formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);

		std::string str = fmt::to_string(formatted);
		ConsoleMessage consoleMsg = { str };
		auto ev = new Events::PrintMessageEvent(consoleMsg);
		auto dispatcher = EngineCore::GetInstance().GetEventDispatcher();
		dispatcher->Dispatch(ev);
	}

	void flush_() override {}
};

void Logger::Initialize(std::string path) {
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::trace);

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	fileSink->set_level(spdlog::level::trace);

	auto editorSink = std::make_shared<EditorSink>();
	editorSink->set_level(spdlog::level::trace);
	editorSink->set_pattern("[%I:%M:%S:%e %p] [%l] %v");

	logger = new spdlog::logger("Debug Logger", { consoleSink, fileSink, editorSink });
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
}

Logger::~Logger() {
	logger->flush();
}
