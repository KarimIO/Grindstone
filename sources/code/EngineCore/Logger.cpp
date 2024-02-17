#include "Logger.hpp"

#include <filesystem>
#include <cstdarg>
#ifdef _MSC_VER
#include <windows.h>
#endif

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/common.h"

#include "Common/Event/PrintMessageEvent.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore.hpp"
using namespace Grindstone;

spdlog::logger *Logger::logger;

LogSeverity SpdLogLevelToGrindstoneLevel(const spdlog::level::level_enum level) {
	switch (level) {
	case spdlog::level::level_enum::info:
		return LogSeverity::Info;
	case spdlog::level::level_enum::warn:
		return LogSeverity::Warning;
	case spdlog::level::level_enum::err:
	case spdlog::level::level_enum::critical:
		return LogSeverity::Error;
	default: break;
	}

	return LogSeverity::Trace;
}

class EditorSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	virtual void sink_it_(const spdlog::details::log_msg& msg) override {
		spdlog::memory_buf_t formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);

		const LogSeverity level = SpdLogLevelToGrindstoneLevel(msg.level);
		const std::string str = fmt::to_string(formatted);
		const ConsoleMessage consoleMsg = { str, level };
		Events::BaseEvent* printEvent = new Events::PrintMessageEvent(consoleMsg);
		Events::Dispatcher* dispatcher = EngineCore::GetInstance().GetEventDispatcher();
		dispatcher->Dispatch(printEvent);
	}

	virtual void flush_() override {}
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

void Logger::Initialize(std::filesystem::path path) {
	std::filesystem::create_directories(path.parent_path());

	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(spdlog::level::trace);

	std::string pathAsString = path.string();
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(pathAsString, true);
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
