#include <fmt/format.h>
#include <vector>

#include <Common/Graphics/Formats.hpp>

#include "Output.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <d3d12shader.h>

inline static const char* GetBoolAsString(bool val) {
	return val ? "true" : "false";
}

static void PrintRenderState(std::string& output, const ParseTree::RenderState& renderState) {
	#define INDENTATION "\t\t\t\t"
	output += "\t\t\trenderState {\n";
	output += fmt::format(INDENTATION "primitiveType: {}\n", GetGeometryTypeName(renderState.primitiveType.value()));
	output += fmt::format(INDENTATION "polygonFillMode: {}\n", GetPolygonFillModeName(renderState.polygonFillMode.value()));
	output += fmt::format(INDENTATION "cullMode: {}\n", GetCullModeName(renderState.cullMode.value()));

	output += fmt::format(INDENTATION "depthCompareOp: {}\n", GetCompareOperationName(renderState.depthCompareOp.value()));
	output += fmt::format(INDENTATION "isStencilEnabled: {}\n", GetBoolAsString(renderState.isStencilEnabled.value()));
	output += fmt::format(INDENTATION "isDepthTestEnabled: {}\n", GetBoolAsString(renderState.isDepthTestEnabled.value()));
	output += fmt::format(INDENTATION "isDepthWriteEnabled: {}\n", GetBoolAsString(renderState.isDepthWriteEnabled.value()));
	output += fmt::format(INDENTATION "isDepthBiasEnabled: {}\n", GetBoolAsString(renderState.isDepthBiasEnabled.value()));
	output += fmt::format(INDENTATION "isDepthClampEnabled: {}\n", GetBoolAsString(renderState.isDepthClampEnabled.value()));

	output += fmt::format(INDENTATION "depthBiasConstantFactor: {}\n", renderState.depthBiasConstantFactor.value());
	output += fmt::format(INDENTATION "depthBiasSlopeFactor: {}\n", renderState.depthBiasSlopeFactor.value());
	output += fmt::format(INDENTATION "depthBiasClamp: {}\n", renderState.depthBiasClamp.value());

	output += INDENTATION "attachments: [\n";
	for (const ParseTree::RenderState::AttachmentData& attachment : renderState.attachmentData) {
		output += INDENTATION "\t{";
		output += fmt::format(INDENTATION "\t\tcolorMask: {}\n", GetColorMaskName(attachment.colorMask.value()));

		output += fmt::format(INDENTATION "\t\tblendColor: {}, {}, {}\n",
			GetBlendOperationName(attachment.blendColorOperation.value()),
			GetBlendFactorName(attachment.blendColorFactorSrc.value()),
			GetBlendFactorName(attachment.blendColorFactorDst.value()));

		output += fmt::format(INDENTATION "\t\tblendAlpha: {}, {}, {}\n",
			GetBlendOperationName(attachment.blendAlphaOperation.value()),
			GetBlendFactorName(attachment.blendAlphaFactorSrc.value()),
			GetBlendFactorName(attachment.blendAlphaFactorDst.value()));

		output += INDENTATION "\t}";
	}

	output += INDENTATION "]\n";
	output += "\t\t\t}\n";
	#undef INDENTATION
}

static const size_t GetTotalSize(const CompilationArtifactsGraphics& compilationArtifacts) {
	size_t totalSize = 0;
	for (const auto& configIterator : compilationArtifacts.configurations) {
		for (const auto& passIterator : configIterator.second.passes) {
			for (const StageCompilationArtifacts& stage : passIterator.second.stages) {
				totalSize += stage.compiledCode.size();
			}
		}
	}

	return totalSize;
}

struct Writer {
	std::vector<char>& outputBuffer;
	char* ptr = outputBuffer.data();
};

static void WriteBytes(Writer& writer, const void* bytes, const size_t count) {
	memcpy(writer.ptr, bytes, count);
	writer.ptr += count;
}

struct PipelineHeader {
	PipelineType isComputeHeader = PipelineType::Graphics;
	uint32_t pipelineSize = 0;
	uint32_t configurationCount = 0;
	uint32_t nextPipelineOffset = 0;
};

struct PipelineConfigurationHeader {
	uint32_t firstPassOffset = 0;
	uint32_t nextConfigurationOffset = 0;
};

struct PipelinePassHeader {
	uint16_t firstStageIndex = 0;
	uint8_t stageCount = 0;
};

struct PipelineStageHeader {
	uint16_t nextStageIndex = 0;
	Grindstone::GraphicsAPI::ShaderStage stageType;
};

struct PipelineSetHeader {
	uint8_t versionMajor = 1;
	uint8_t versionMinor = 0;
	uint8_t versionPatch = 0;
	uint8_t headerSize = sizeof(PipelineSetHeader);
	uint8_t pipelineSize = sizeof(PipelineSetHeader);
	uint8_t configurationSize = sizeof(PipelineSetHeader);
	uint8_t passSize = sizeof(PipelineSetHeader);
	uint32_t pipelineCount = 0;
};

bool OutputPipelineSet(LogCallback logCallback, const CompilationArtifactsGraphics& compilationArtifacts, const ResolvedStateTree::PipelineSet& pipelineSet, WriteCallback writeCallback) {
	const size_t totalSize = GetTotalSize(compilationArtifacts);

	if (totalSize == 0) {
		return false;
	}

	std::vector<char> outputBuffer;
	outputBuffer.resize(totalSize);

	Writer writer{ outputBuffer };
	writer.ptr = outputBuffer.data();
	WriteBytes(writer, "GSHAD", 4);

	PipelineSetHeader pipelineSetHeader;
	pipelineSetHeader.pipelineCount = 1;
	
	std::vector<PipelineConfigurationHeader> configurations;
	
	bool hasUsedConfigs = false;
	for (const auto& configIterator : pipelineSet.configurations) {

		const ResolvedStateTree::Configuration& config = configIterator.second;
		const auto& configArtifactIterator = compilationArtifacts.configurations.find(configIterator.first);
		if (configArtifactIterator == compilationArtifacts.configurations.end()) {
			continue;
		}

		const CompilationArtifactsGraphics::Configuration& configArtifact = configArtifactIterator->second;
		
		for (std::string_view tag : configIterator.second.tags) {
			
		}
		
		bool hasUsedPasses = false;
		for (const auto& passIterator : config.passes) {
			const ResolvedStateTree::Pass& pass = passIterator.second;
			const auto& passArtifactsIterator = configArtifact.passes.find(passIterator.first);
			if (passArtifactsIterator == configArtifact.passes.end()) {
				continue;
			}
			
			const CompilationArtifactsGraphics::Pass& passArtifact = passArtifactsIterator->second;
			
			bool hasUsedStages = false;
			size_t usedStageIndex = 0;
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				const std::string& stageEntrypoint = pass.stageEntryPoints[stageIndex];
				if (!stageEntrypoint.empty()) {
					if (usedStageIndex >= passArtifact.stages.size()) {
						continue;
					}

					const StageCompilationArtifacts& artifacts = passArtifact.stages[usedStageIndex++];
					if (artifacts.stage != static_cast<Grindstone::GraphicsAPI::ShaderStage>(stageIndex)) {
						logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Output, "Something fishy - stage mismatch!", pipelineSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
					}
					hasUsedStages = true;
					hasUsedPasses = true;
					hasUsedConfigs = true;
				}
			}
		}
	}

	// writeCallback(pipelineSet.sourceFilepath, outputBuffer.size(), outputBuffer.data());

	return true;
}

bool OutputComputeSet(LogCallback logCallback, const CompilationArtifactsCompute& compilationArtifacts, const ResolvedStateTree::ComputeSet& computeSet, WriteCallback writeCallback) {
	logCallback(Grindstone::LogSeverity::Info, PipelineConverterLogSource::Output, "Compiled and Outputted", computeSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);

	return true;
}
