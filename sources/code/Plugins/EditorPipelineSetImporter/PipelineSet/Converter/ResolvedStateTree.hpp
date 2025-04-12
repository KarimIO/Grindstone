#pragma once

#include <set>
#include <map>
#include <array>
#include <vector>

#include <Common/Graphics/Formats.hpp>
#include "ParseTree.hpp"

struct ResolvedStateTree {
	struct Pass {
		std::string name;
		ParseTree::RenderState renderState;
		std::string code;
		ShaderCodeType type = ShaderCodeType::Unset;
		std::set<std::string_view> requiredShaderBlocks; // Only used in resolve step
		std::array<std::string, Grindstone::GraphicsAPI::numShaderTotalStage> stageEntryPoints;
	};

	struct Configuration {
		std::string name;
		std::set<std::string> tags;
		std::map<std::string, Pass> passes;
	};

	struct PipelineSet {
		std::filesystem::path sourceFilepath;
		std::string name;
		std::map<std::string, Configuration> configurations;
	};

	struct ComputeSet {
		std::filesystem::path sourceFilepath;
		std::string name;
		std::string shaderCode;
		std::string shaderEntrypoint;
		ShaderCodeType shaderType = ShaderCodeType::Unset;
		std::set<std::string_view> requiredShaderBlocks; // Only used in resolve step
	};

	std::vector<PipelineSet> pipelineSets;
	std::vector<ComputeSet> computeSets;
};
