#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

enum class LogLevel : uint8_t {
	Trace,
	Info,
	Warning,
	Error,
	Fatal
};

enum class LogSource : uint8_t {
	General,
	Scanner,
	Parser,
	Resolver,
	Output
};

constexpr uint32_t UNDEFINED_LINE = UINT32_MAX;
constexpr uint32_t UNDEFINED_COLUMN = UINT32_MAX;

using LogCallback = std::function<void(LogLevel level, LogSource source, std::string_view msg, const std::filesystem::path& filename, uint32_t line, uint32_t column)>;
