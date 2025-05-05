#include <fmt/format.h>
#include <vector>

#include <Common/Graphics/Formats.hpp>
#include <Common/Formats/PipelineSet.hpp>

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

struct DescriptorSetOutput {
	uint32_t setIndex;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding> bindings;
};

static void ConsolidateDescriptorSets(
	Grindstone::GraphicsAPI::ShaderStageBit stageBit,
	std::vector<DescriptorSetOutput>& passDescriptorSets,
	const std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet>& stageDescriptorSets,
	const std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding>& stageDescriptorBindings
) {
	for (size_t i = 0; i < stageDescriptorSets.size(); ++i) {
		const Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet& srcSet = stageDescriptorSets[i];

		size_t dstIndex = 0;
		for (; dstIndex < passDescriptorSets.size(); ++dstIndex) {
			if (passDescriptorSets[dstIndex].setIndex == srcSet.setIndex) {
				break;
			}
		}

		if (dstIndex == passDescriptorSets.size()) {
			passDescriptorSets.emplace_back();
		}

		DescriptorSetOutput& dstSet = passDescriptorSets[dstIndex];
		dstSet.setIndex = srcSet.setIndex;

		for (size_t srcBindingIndex = srcSet.firstBindingIndex;
			srcBindingIndex < srcSet.firstBindingIndex + srcSet.bindingCount;
			++srcBindingIndex
		) {
			const Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& srcBinding = stageDescriptorBindings[srcBindingIndex];

			bool hasFoundBinding = false;
			for (Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& dstBinding : dstSet.bindings) {
				if (srcBinding.bindingIndex == dstBinding.bindingIndex) {
					dstBinding.stages |= srcBinding.stages;
					hasFoundBinding = true;
					break;
				}
			}

			if (!hasFoundBinding) {
				Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& newBinding =
					dstSet.bindings.emplace_back();

				newBinding.bindingIndex = srcBinding.bindingIndex;
				newBinding.count = srcBinding.count;
				newBinding.stages = srcBinding.stages;
				newBinding.type = srcBinding.type;
			}
		}
	}
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

bool OutputPipelineSet(LogCallback logCallback, const CompilationArtifactsGraphics& compilationArtifacts, const ResolvedStateTree::PipelineSet& pipelineSet, PipelineOutput& outputFile) {
	const size_t totalSize = GetTotalSize(compilationArtifacts);

	if (totalSize == 0) {
		return false;
	}

	outputFile.name = pipelineSet.name;
	outputFile.pipelineType = Grindstone::Formats::Pipelines::V1::PipelineType::Graphics;
	std::vector<char>& outputBuffer = outputFile.content;
	outputBuffer.resize(totalSize);

	Writer writer{ outputBuffer };
	WriteBytes(writer, Grindstone::Formats::Pipelines::V1::FileMagicCode, 4);

	// ========================================
	// Write File headers
	// ========================================
	Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader pipelineSetFileHeader{};
	pipelineSetFileHeader.graphicsPipelineCount = 1;
	pipelineSetFileHeader.computePipelineCount = 0;
	WriteBytes(writer, &pipelineSetFileHeader, sizeof(pipelineSetFileHeader));

	// ========================================
	// Write Pipeline headers
	// ========================================
	Grindstone::Formats::Pipelines::V1::GraphicsPipelineHeader pipelineSetHeader{};
	pipelineSetHeader.configurationCount = static_cast<uint32_t>(pipelineSet.configurations.size());
	WriteBytes(writer, &pipelineSetHeader, sizeof(pipelineSetHeader));

	// ========================================
	// Write Configuration headers
	// ========================================
	uint32_t passCount = 0;
	std::vector<Grindstone::Formats::Pipelines::V1::PipelineConfigurationHeader> configHeaders;
	configHeaders.reserve(pipelineSetHeader.configurationCount);
	for (auto& [configName, srcConfig] : pipelineSet.configurations) {
		Grindstone::Formats::Pipelines::V1::PipelineConfigurationHeader& configHeader = configHeaders.emplace_back();
		configHeader.passCount = static_cast<uint32_t>(pipelineSet.configurations.size());
		passCount += configHeader.passCount;
	}
	WriteBytes(writer, configHeaders.data(), sizeof(Grindstone::Formats::Pipelines::V1::PipelineConfigurationHeader) * pipelineSetHeader.configurationCount);

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

			std::vector<DescriptorSetOutput> passDescriptorSets;

			uint8_t shaderStageCount = 0;
			std::array<uint32_t, Grindstone::GraphicsAPI::numShaderTotalStage> shaderCodeSizes{};
			size_t usedStageIndex = 0;
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				const std::string& stageEntrypoint = pass.stageEntryPoints[stageIndex];
				if (!stageEntrypoint.empty()) {
					if (usedStageIndex >= passArtifact.stages.size()) {
						continue;
					}

					const StageCompilationArtifacts& artifacts = passArtifact.stages[usedStageIndex++];
					shaderCodeSizes[stageIndex] = static_cast<uint32_t>(artifacts.compiledCode.size());

					if (shaderCodeSizes[stageIndex] != 0) {
						Grindstone::GraphicsAPI::ShaderStageBit stageBit =
							static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(1 << stageIndex);

						ConsolidateDescriptorSets(
							stageBit,
							passDescriptorSets,
							artifacts.reflectedDescriptorSets,
							artifacts.reflectedDescriptorBindings
						);

						shaderStageCount++;
					}
				}
			}

			std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet> passOutDescriptorSets;
			std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding> passOutDescriptorBindings;
			passOutDescriptorSets.resize(passDescriptorSets.size());
			for (DescriptorSetOutput& srcSet : passDescriptorSets) {
				size_t firstIndex = passOutDescriptorBindings.size();
				size_t bindingCount = srcSet.bindings.size();

				for (Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& srcBinding : srcSet.bindings) {
					passOutDescriptorBindings.emplace_back(srcBinding);
				}

				Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet& dstSet = passOutDescriptorSets.emplace_back();
				dstSet.setIndex = srcSet.setIndex;
				dstSet.firstBindingIndex = static_cast<uint32_t>(firstIndex);
				dstSet.bindingCount = static_cast<uint32_t>(bindingCount);
			}

			uint8_t flags = 0;
			flags |= pass.renderState.isDepthBiasEnabled.value() ? 0b1 : 0;
			flags |= pass.renderState.isDepthClampEnabled.value() ? 0b10 : 0;
			flags |= pass.renderState.isDepthTestEnabled.value() ? 0b100 : 0;
			flags |= pass.renderState.isDepthWriteEnabled.value() ? 0b1000 : 0;
			flags |= pass.renderState.isStencilEnabled.value() ? 0b10000 : 0;
			flags |= pass.renderState.shouldCopyFirstAttachment ? 0b100000 : 0;

			Grindstone::Formats::Pipelines::V1::PassPipelineHeader passPipelineHeader{};
			passPipelineHeader.depthBiasClamp = pass.renderState.depthBiasClamp.value();
			passPipelineHeader.depthBiasConstantFactor = pass.renderState.depthBiasConstantFactor.value();
			passPipelineHeader.depthBiasSlopeFactor = pass.renderState.depthBiasSlopeFactor.value();
			passPipelineHeader.cullMode = pass.renderState.cullMode.value();
			passPipelineHeader.depthCompareOp = pass.renderState.depthCompareOp.value();
			passPipelineHeader.polygonFillMode = pass.renderState.polygonFillMode.value();
			passPipelineHeader.primitiveType = pass.renderState.primitiveType.value();
			passPipelineHeader.flags = flags;
			passPipelineHeader.attachmentCount = static_cast<uint8_t>(pass.renderState.attachmentData.size());
			passPipelineHeader.shaderStageCount = shaderStageCount;
			passPipelineHeader.descriptorSetCount = static_cast<uint8_t>(passOutDescriptorSets.size());
			passPipelineHeader.descriptorBindingCount = static_cast<uint8_t>(passOutDescriptorBindings.size());
			WriteBytes(writer, &passPipelineHeader, sizeof(Grindstone::Formats::Pipelines::V1::PassPipelineHeader));

			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				if (shaderCodeSizes[stageIndex] != 0) {
					Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader stageHeader{};
					stageHeader.shaderCodeSize = shaderCodeSizes[stageIndex];
					stageHeader.stageType = static_cast<Grindstone::GraphicsAPI::ShaderStage>(stageIndex);

					WriteBytes(writer, &stageHeader, sizeof(Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader));
				}
			}

			for (const ParseTree::RenderState::AttachmentData& attachmentData : pass.renderState.attachmentData) {
				Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader attachmentHeader{};
				attachmentHeader.colorMask = attachmentData.colorMask.value();
				attachmentHeader.blendAlphaFactorDst = attachmentData.blendAlphaFactorDst.value();
				attachmentHeader.blendAlphaFactorSrc = attachmentData.blendAlphaFactorSrc.value();
				attachmentHeader.blendAlphaOperation = attachmentData.blendAlphaOperation.value();
				attachmentHeader.blendColorFactorDst = attachmentData.blendColorFactorDst.value();
				attachmentHeader.blendColorFactorSrc = attachmentData.blendColorFactorSrc.value();
				attachmentHeader.blendColorOperation = attachmentData.blendColorOperation.value();
				WriteBytes(writer, &attachmentHeader, sizeof(Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader));
			}

			WriteBytes(writer, passOutDescriptorSets.data(), passPipelineHeader.descriptorSetCount * sizeof(Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet));
			WriteBytes(writer, passOutDescriptorBindings.data(), passPipelineHeader.descriptorBindingCount * sizeof(Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding));

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
	outputFile.pipelineType = Grindstone::Formats::Pipelines::V1::PipelineType::Compute;
	outputFile.content.resize(compilationArtifacts.computeStage.compiledCode.size() + 4);

	Writer writer{ outputFile.content };
	WriteBytes(writer, Grindstone::Formats::Pipelines::V1::FileMagicCode, 4);

	Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader pipelineSetHeader{};
	pipelineSetHeader.graphicsPipelineCount = 0;
	pipelineSetHeader.computePipelineCount = 1;

	Grindstone::Formats::Pipelines::V1::ComputePipelineHeader computeHeader{};
	computeHeader.codeSize = static_cast<uint32_t>(compilationArtifacts.computeStage.compiledCode.size());

	WriteBytes(writer, &pipelineSetHeader, sizeof(pipelineSetHeader));
	WriteBytes(writer, &computeHeader, sizeof(computeHeader));
	WriteBytes(writer, compilationArtifacts.computeStage.compiledCode.data(), compilationArtifacts.computeStage.compiledCode.size());

	return true;
}
