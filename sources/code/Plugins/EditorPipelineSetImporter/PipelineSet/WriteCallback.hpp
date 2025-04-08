#pragma once

#include <filesystem>
#include <string_view>
#include <functional>

enum class PipelineType {
	Graphics,
	Compute
};

struct PipelineOutput {
	PipelineType pipelineType;
	std::string_view name;
	void* content;
	size_t size;
};

using WriteCallback = std::function<void(const std::filesystem::path& path, const std::vector<PipelineOutput>& pipelines)>;
