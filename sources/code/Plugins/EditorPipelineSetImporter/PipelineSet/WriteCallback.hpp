#pragma once

#include <filesystem>
#include <string_view>
#include <functional>
#include <vector>

#include <Common/Formats/PipelineSet.hpp>

struct PipelineOutput {
	Grindstone::Formats::Pipelines::V1::PipelineType pipelineType;
	std::string_view name;
	std::vector<char> content;
};

using WriteCallback = std::function<void(const std::filesystem::path& path, const std::vector<PipelineOutput>& pipelines)>;
using ResolvePathCallback = std::function<std::filesystem::path(const std::filesystem::path& path)>;
