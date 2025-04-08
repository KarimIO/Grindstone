#pragma once

#include <filesystem>
#include <vector>
#include <set>
#include <map>

#include <PipelineSet/Converter/CompilationOptions.hpp>
#include <PipelineSet/WriteCallback.hpp>
#include <PipelineSet/Log.hpp>

class PipelineSetConditioner {
public:
	PipelineSetConditioner(WriteCallback writeFn = nullptr, LogCallback logFn = nullptr);
	void Add(const std::filesystem::path& path);
	void Scan(const std::filesystem::path& path);
	void Convert(CompilationOptions options);
	void Rerun(const std::filesystem::path& path);
	LogCallback GetLogCallback() const;

private:
	std::set<std::filesystem::path> unprocessedFiles;

	struct ShaderBlock {
		std::filesystem::path srcPath;
		std::string code;
	};

	std::map<std::string, ShaderBlock> shaderBlocks;

	struct PipelineTemplate {
		std::filesystem::path srcPath;
		std::vector<std::filesystem::path> requiredBlocks;
	};

	std::map<std::string, PipelineTemplate> pipelineTemplates;

	WriteCallback writeFileCallback = nullptr;
	LogCallback logCallback = nullptr;
};
