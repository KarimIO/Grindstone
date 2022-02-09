#include "Logger.hpp"

#include <iostream>
#include <filesystem>
#include <stdarg.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
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

LogSeverity SpdLogLevelToGrindstoneLevel(spdlog::level::level_enum level) {
	switch (level) {
	default:
	case spdlog::level::level_enum::trace:
		return LogSeverity::Trace;
	case spdlog::level::level_enum::debug:
		return LogSeverity::Trace;
	case spdlog::level::level_enum::info:
		return LogSeverity::Info;
	case spdlog::level::level_enum::warn:
		return LogSeverity::Warning;
	case spdlog::level::level_enum::err:
		return LogSeverity::Error;
	case spdlog::level::level_enum::critical:
		return LogSeverity::Error;
	}
}

class EditorSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		spdlog::memory_buf_t formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);

		auto level = SpdLogLevelToGrindstoneLevel(msg.level);
		std::string str = fmt::to_string(formatted);
		ConsoleMessage consoleMsg = { str, level };
		auto ev = new Events::PrintMessageEvent(consoleMsg);
		auto dispatcher = EngineCore::GetInstance().GetEventDispatcher();
		dispatcher->Dispatch(ev);
	}

	void flush_() override {}
};

#ifdef _MSC_VER
class VisualStudioOutputSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		spdlog::memory_buf_t formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);

		std::string str = fmt::to_string(formatted);
#ifdef OutputDebugString
		OutputDebugString(str.c_str());
#endif
	}

	void flush_() override {}
};
#endif

void Logger::Initialize(std::string path) {
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::trace);

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
	fileSink->set_level(spdlog::level::trace);

	auto editorSink = std::make_shared<EditorSink>();
	editorSink->set_level(spdlog::level::trace);
	editorSink->set_pattern("[%I:%M:%S:%e %p] [%l] %v");

#ifdef _MSC_VER
	auto visualStudioOutputSink = std::make_shared<VisualStudioOutputSink>();
	visualStudioOutputSink->set_level(spdlog::level::trace);
	visualStudioOutputSink->set_pattern("[%I:%M:%S:%e %p] [%l] %v");
#endif

	logger = new spdlog::logger("Debug Logger", {
		consoleSink,
		fileSink,
		editorSink,
#ifdef _MSC_VER
		visualStudioOutputSink
#endif
	});
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
}

void Logger::Print(LogSeverity logSeverity, const char* str) {
	switch (logSeverity) {
	case LogSeverity::Info:
		logger->info(str);
		break;
	case LogSeverity::Trace:
		logger->trace(str);
		break;
	case LogSeverity::Warning:
		logger->warn(str);
		break;
	case LogSeverity::Error:
		logger->error(str);
		break;
	}
}

Logger::~Logger() {
	logger->flush();
}
