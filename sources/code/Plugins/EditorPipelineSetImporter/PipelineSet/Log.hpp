#pragma once

#include <filesystem>
#include <functional>
#include <string_view>
#include <stdint.h>

#include <Common/Logging.hpp>

enum class PipelineConverterLogSource : uint8_t {
	General,
	Scanner,
	Parser,
	Resolver,
	Output
};

constexpr uint32_t UNDEFINED_LINE = UINT32_MAX;
constexpr uint32_t UNDEFINED_COLUMN = UINT32_MAX;

// Grindstone::LogSeverity level, PipelineConverterLogSource source, std::string_view msg, const std::filesystem::path& filename, uint32_t line, uint32_t column
using LogCallback = std::function<void(Grindstone::LogSeverity, PipelineConverterLogSource, std::string_view, const std::filesystem::path&, uint32_t, uint32_t)>;
