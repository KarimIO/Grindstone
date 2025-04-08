#pragma once

#include <map>
#include <string_view>
#include <vector>

#include <Common/Graphics/Formats.hpp>

struct StageCompilationArtifacts {
	Grindstone::GraphicsAPI::ShaderStage stage;
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
