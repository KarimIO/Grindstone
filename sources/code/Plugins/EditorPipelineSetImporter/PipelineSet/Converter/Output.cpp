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
	size_t offset = 0;
};

static void WriteBytes(Writer& writer, const void* bytes, const size_t count) {
	if (writer.offset + count >= writer.outputBuffer.size()) {
		writer.outputBuffer.resize(writer.offset + count);
	}

	memcpy(writer.outputBuffer.data() + writer.offset, bytes, count);
	writer.offset += count;
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

bool OutputPipelineSet(LogCallback logCallback, const CompilationArtifactsGraphics& compilationArtifacts, const ResolvedStateTree::PipelineSet& pipelineSet, PipelineOutput& outputFile) {
	const size_t totalSize = GetTotalSize(compilationArtifacts);

	if (totalSize == 0) {
		return false;
	}

	outputFile.name = pipelineSet.name;
	outputFile.pipelineType = PipelineType::Graphics;
	std::vector<char>& outputBuffer = outputFile.content;
	outputBuffer.resize(totalSize);

	Writer writer{ outputBuffer };
	WriteBytes(writer, "GGP", 4);

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

			uint8_t flags = 0;
			flags |= pass.renderState.isDepthBiasEnabled.value() ? 0b1 : 0;
			flags |= pass.renderState.isDepthClampEnabled.value() ? 0b10 : 0;
			flags |= pass.renderState.isDepthTestEnabled.value() ? 0b100 : 0;
			flags |= pass.renderState.isDepthWriteEnabled.value() ? 0b1000 : 0;
			flags |= pass.renderState.isStencilEnabled.value() ? 0b10000 : 0;
			flags |= pass.renderState.shouldCopyFirstAttachment ? 0b100000 : 0;

			const CompilationArtifactsGraphics::Pass& passArtifact = passArtifactsIterator->second;
			WriteBytes(writer, &pass.renderState.depthBiasClamp.value(), sizeof(float));
			WriteBytes(writer, &pass.renderState.depthBiasConstantFactor.value(), sizeof(float));
			WriteBytes(writer, &pass.renderState.depthBiasSlopeFactor.value(), sizeof(float));
			WriteBytes(writer, &pass.renderState.cullMode.value(), sizeof(Grindstone::GraphicsAPI::CullMode));
			WriteBytes(writer, &pass.renderState.depthCompareOp.value(), sizeof(Grindstone::GraphicsAPI::CompareOperation));
			WriteBytes(writer, &pass.renderState.polygonFillMode.value(), sizeof(Grindstone::GraphicsAPI::PolygonFillMode));
			WriteBytes(writer, &pass.renderState.primitiveType.value(), sizeof(Grindstone::GraphicsAPI::GeometryType));
			WriteBytes(writer, &flags, sizeof(uint8_t));

			uint8_t attachmentCount = static_cast<uint8_t>(pass.renderState.attachmentData.size());

			uint8_t shaderStageCount = 0;
			std::array<uint32_t, Grindstone::GraphicsAPI::numShaderTotalStage> shaderCodeSizes;
			size_t usedStageIndex = 0;
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				const std::string& stageEntrypoint = pass.stageEntryPoints[stageIndex];
				if (!stageEntrypoint.empty()) {
					if (usedStageIndex >= passArtifact.stages.size()) {
						continue;
					}

					const StageCompilationArtifacts& artifacts = passArtifact.stages[usedStageIndex++];
					shaderCodeSizes[stageIndex] = artifacts.compiledCode.size();

					if (shaderCodeSizes[stageIndex] != 0) {
						shaderStageCount++;
					}
				}
			}

			static_assert(sizeof(Grindstone::GraphicsAPI::ShaderStage) == sizeof(uint8_t));
			WriteBytes(writer, &shaderStageCount, sizeof(shaderStageCount));
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				if (shaderCodeSizes[stageIndex] != 0) {
					WriteBytes(writer, &stageIndex, sizeof(uint8_t));
					WriteBytes(writer, &shaderCodeSizes[stageIndex], sizeof(uint32_t));
				}
			}


			WriteBytes(writer, &attachmentCount, sizeof(uint8_t));
			for (const ParseTree::RenderState::AttachmentData& attachmentData : pass.renderState.attachmentData) {
				WriteBytes(writer, &attachmentData.colorMask, sizeof(attachmentData.colorMask));
				WriteBytes(writer, &attachmentData.blendAlphaFactorDst, sizeof(attachmentData.blendAlphaFactorDst));
				WriteBytes(writer, &attachmentData.blendAlphaFactorSrc, sizeof(attachmentData.blendAlphaFactorSrc));
				WriteBytes(writer, &attachmentData.blendAlphaOperation, sizeof(attachmentData.blendAlphaOperation));
				WriteBytes(writer, &attachmentData.blendColorFactorDst, sizeof(attachmentData.blendColorFactorDst));
				WriteBytes(writer, &attachmentData.blendColorFactorSrc, sizeof(attachmentData.blendColorFactorSrc));
				WriteBytes(writer, &attachmentData.blendColorOperation, sizeof(attachmentData.blendColorOperation));
			}

			bool hasUsedStages = false;
			usedStageIndex = 0;
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

					uint32_t shaderCodeSize = static_cast<uint32_t>(artifacts.compiledCode.size());
					WriteBytes(writer, artifacts.compiledCode.data(), shaderCodeSize);
					hasUsedStages = true;
					hasUsedPasses = true;
					hasUsedConfigs = true;
				}
			}
		}
	}


	return true;
}

bool OutputComputeSet(LogCallback logCallback, const CompilationArtifactsCompute& compilationArtifacts, const ResolvedStateTree::ComputeSet& computeSet, PipelineOutput& outputFile) {
	logCallback(Grindstone::LogSeverity::Info, PipelineConverterLogSource::Output, "Compiled and Outputted", computeSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
	outputFile.name = computeSet.name;
	outputFile.pipelineType = PipelineType::Compute;
	outputFile.content.resize(compilationArtifacts.computeStage.compiledCode.size() + 4);
	Writer writer{ outputFile.content };
	WriteBytes(writer, "GCP", 4);
	memcpy(outputFile.content.data(), compilationArtifacts.computeStage.compiledCode.data(), compilationArtifacts.computeStage.compiledCode.size());

	return true;
}
