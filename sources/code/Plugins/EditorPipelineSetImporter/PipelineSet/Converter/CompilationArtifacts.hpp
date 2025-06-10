#pragma once

#include <map>
#include <string_view>
#include <string>
#include <vector>

#include <Common/Graphics/Formats.hpp>
#include <Common/Formats/PipelineSet.hpp>

struct ReflectedBufferBinding {
	uint32_t setIndex;
	uint32_t bindingIndex;
	uint32_t blockIndex;
};

struct ReflectedBlock {
	std::string name;
	uint32_t startVariableIndex;
	uint32_t variableCount;
	uint32_t totalSize;
};

struct ReflectedBlockVariable {
	std::string name;
	uint32_t offset;
	uint32_t size;
	Grindstone::Formats::Pipelines::V1::ReflectedBlockVariableType type;
};


struct StageCompilationArtifacts {
	Grindstone::GraphicsAPI::ShaderStage stage;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding> reflectedDescriptorBindings;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet> reflectedDescriptorSets;
	std::vector<ReflectedBufferBinding> reflectedBufferBindings;
	std::vector<ReflectedBlock> reflectedBlocks;
	std::vector<ReflectedBlockVariable> reflectedBlockVariables;
	std::vector<Grindstone::Formats::Pipelines::V1::MaterialResource> materialResources;
	std::vector<char> hash;
	std::vector<char> compiledCode;
	std::vector<char> compiledPdb;
};

struct CompilationArtifactsGraphics {
	struct Pass {
		std::vector<StageCompilationArtifacts> stages;
	};

	struct Configuration {
		std::map<std::string_view, Pass> passes;
	};

	std::map<std::string_view, Configuration> configurations;
};

struct CompilationArtifactsCompute {
	StageCompilationArtifacts computeStage;
};
