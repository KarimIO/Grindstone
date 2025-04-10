#pragma once

#include <filesystem>
#include <string_view>
#include <functional>
#include <vector>

enum class PipelineType {
	Graphics,
	Compute
};

struct PipelineOutput {
	PipelineType pipelineType;
	std::string_view name;
	std::vector<char> content;
};

using WriteCallback = std::function<void(const std::filesystem::path& path, const std::vector<PipelineOutput>& pipelines)>;
using ResolvePathCallback = std::function<std::filesystem::path(const std::filesystem::path& path)>;
