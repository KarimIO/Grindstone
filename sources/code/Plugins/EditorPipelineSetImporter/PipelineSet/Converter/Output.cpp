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

		size_t bindingEnd = static_cast<size_t>(srcSet.bindingStartIndex) + srcSet.bindingCount;
		for (size_t srcBindingIndex = srcSet.bindingStartIndex;
			srcBindingIndex < bindingEnd;
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

static void WriteBytes(Writer& writer, const void* bytes, const size_t size) {
	if (writer.offset + size >= writer.outputBuffer.size()) {
		writer.outputBuffer.resize(writer.offset + size);
	}

	memcpy(writer.outputBuffer.data() + writer.offset, bytes, size);
	writer.offset += size;
}

template<typename T>
static void WriteBytes(Writer& writer, const std::vector<T>& list) {
	WriteBytes(writer, list.data(), list.size() * sizeof(T));
}

static inline uint32_t IncrementSize(uint32_t& offset, uint32_t size) {
	uint32_t preOffset = offset;
	offset += size;
	return preOffset;
}

static inline uint32_t IncrementSize(uint32_t& offset, uint32_t size, size_t count) {
	uint32_t preOffset = offset;
	offset += size * static_cast<uint32_t>(count);
	return preOffset;
}

template<typename T>
static inline void AssignCountAndOffsetAndIncrementSize(uint32_t& totalSize, uint32_t& offset, uint32_t& count, const std::vector<T>& list) {
	offset = IncrementSize(totalSize, sizeof(T), list.size());
	count = static_cast<uint32_t>(list.size());
}

static void ExtractGraphicsPipelineSet(
	LogCallback logCallback,
	const CompilationArtifactsGraphics& compilationArtifacts,
	const ResolvedStateTree::PipelineSet& pipelineSet,
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineSetHeader>& pipelineSetHeaders,
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineConfigurationHeader>& configHeaders,
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineHeader>& passHeaders,
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader>& shaderStages,
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader>& attachmentHeaders,
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet>& descriptorSets,
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding>& descriptorBindings,
	std::vector<Grindstone::Formats::Pipelines::V1::MaterialParameter>& materialParameters,
	std::vector<Grindstone::Formats::Pipelines::V1::MaterialResource>& materialResources,
	Writer& blobWriter
) {
	// ========================================
	// Write Pipeline headers
	// ========================================
	Grindstone::Formats::Pipelines::V1::GraphicsPipelineSetHeader& pipelineSetHeader = pipelineSetHeaders.emplace_back();
	pipelineSetHeader.configurationStartIndex = static_cast<uint32_t>(configHeaders.size());
	pipelineSetHeader.configurationCount = static_cast<uint32_t>(pipelineSet.configurations.size());

	bool hasUsedConfigs = false;
	for (const auto& configIterator : pipelineSet.configurations) {
		const ResolvedStateTree::Configuration& config = configIterator.second;
		const auto& configArtifactIterator = compilationArtifacts.configurations.find(configIterator.first);
		if (configArtifactIterator == compilationArtifacts.configurations.end()) {
			continue;
		}

		const CompilationArtifactsGraphics::Configuration& configArtifact = configArtifactIterator->second;

		auto& dstConfigurationHeader = configHeaders.emplace_back();
		dstConfigurationHeader.passStartIndex = static_cast<uint32_t>(passHeaders.size());
		dstConfigurationHeader.passCount = static_cast<uint32_t>(config.passes.size());

		bool hasUsedPasses = false;
		for (const auto& passIterator : config.passes) {
			const ResolvedStateTree::Pass& pass = passIterator.second;
			const auto& passArtifactsIterator = configArtifact.passes.find(passIterator.first);
			if (passArtifactsIterator == configArtifact.passes.end()) {
				continue;
			}

			const CompilationArtifactsGraphics::Pass& passArtifact = passArtifactsIterator->second;
			Grindstone::Formats::Pipelines::V1::PassPipelineHeader& passPipelineHeader = passHeaders.emplace_back();

			passPipelineHeader.pipelineNameOffsetFromBlobStart = static_cast<uint32_t>(blobWriter.offset);
			WriteBytes(blobWriter, pass.name.data(), pass.name.size() + 1); // +1 for null-terminated

			passPipelineHeader.renderQueueNameOffsetFromBlobStart = static_cast<uint32_t>(blobWriter.offset);
			WriteBytes(blobWriter, pass.renderQueue.data(), pass.renderQueue.size() + 1); // +1 for null-terminated

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

			uint16_t descriptorSetOffset = static_cast<uint16_t>(descriptorSets.size());
			uint16_t descriptorBindingOffset = static_cast<uint16_t>(descriptorBindings.size());

			for (DescriptorSetOutput& srcSet : passDescriptorSets) {
				size_t firstIndex = descriptorBindings.size();
				size_t bindingCount = srcSet.bindings.size();

				for (Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& srcBinding : srcSet.bindings) {
					descriptorBindings.emplace_back(srcBinding);
				}

				Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet& dstSet = descriptorSets.emplace_back();
				dstSet.setIndex = srcSet.setIndex;
				dstSet.bindingStartIndex = static_cast<uint32_t>(firstIndex);
				dstSet.bindingCount = static_cast<uint32_t>(bindingCount);
			}

			uint8_t flags = 0;
			flags |= pass.renderState.isDepthBiasEnabled.value() ? 0b1 : 0;
			flags |= pass.renderState.isDepthClampEnabled.value() ? 0b10 : 0;
			flags |= pass.renderState.isDepthTestEnabled.value() ? 0b100 : 0;
			flags |= pass.renderState.isDepthWriteEnabled.value() ? 0b1000 : 0;
			flags |= pass.renderState.isStencilEnabled.value() ? 0b10000 : 0;
			flags |= pass.renderState.shouldCopyFirstAttachment ? 0b100000 : 0;

			passPipelineHeader.depthBiasClamp = pass.renderState.depthBiasClamp.value();
			passPipelineHeader.depthBiasConstantFactor = pass.renderState.depthBiasConstantFactor.value();
			passPipelineHeader.depthBiasSlopeFactor = pass.renderState.depthBiasSlopeFactor.value();
			passPipelineHeader.cullMode = pass.renderState.cullMode.value();
			passPipelineHeader.depthCompareOp = pass.renderState.depthCompareOp.value();
			passPipelineHeader.polygonFillMode = pass.renderState.polygonFillMode.value();
			passPipelineHeader.primitiveType = pass.renderState.primitiveType.value();
			passPipelineHeader.flags = flags;
			passPipelineHeader.attachmentStartIndex = static_cast<uint16_t>(attachmentHeaders.size());
			passPipelineHeader.attachmentCount = static_cast<uint8_t>(pass.renderState.attachmentData.size());
			passPipelineHeader.shaderStageStartIndex = static_cast<uint16_t>(shaderStages.size());
			passPipelineHeader.shaderStageCount = shaderStageCount;
			passPipelineHeader.descriptorSetStartIndex = static_cast<uint16_t>(descriptorSetOffset);
			passPipelineHeader.descriptorSetCount = static_cast<uint8_t>(descriptorSets.size() - descriptorSetOffset);
			passPipelineHeader.descriptorBindingStartIndex = static_cast<uint16_t>(descriptorBindingOffset);
			passPipelineHeader.descriptorBindingCount = static_cast<uint8_t>(descriptorBindings.size() - descriptorBindingOffset);

			usedStageIndex = 0;
			for (uint8_t stageIndex = 0; stageIndex < Grindstone::GraphicsAPI::numShaderTotalStage; ++stageIndex) {
				if (shaderCodeSizes[stageIndex] != 0) {
					const StageCompilationArtifacts& artifacts = passArtifact.stages[usedStageIndex++];
					if (artifacts.stage != static_cast<Grindstone::GraphicsAPI::ShaderStage>(stageIndex)) {
						logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::Output, "Something fishy - stage mismatch!", pipelineSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
					}

					Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader& stageHeader = shaderStages.emplace_back();
					stageHeader.shaderCodeSize = shaderCodeSizes[stageIndex];
					stageHeader.stageType = static_cast<Grindstone::GraphicsAPI::ShaderStage>(stageIndex);
					stageHeader.shaderCodeOffsetFromBlobStart = static_cast<uint32_t>(blobWriter.offset);
					WriteBytes(blobWriter, artifacts.compiledCode.data(), stageHeader.shaderCodeSize);
				}
			}

			for (const ParseTree::RenderState::AttachmentData& attachmentData : pass.renderState.attachmentData) {
				Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader& attachmentHeader = attachmentHeaders.emplace_back();
				attachmentHeader.colorMask = attachmentData.colorMask.value();
				attachmentHeader.blendAlphaFactorDst = attachmentData.blendAlphaFactorDst.value();
				attachmentHeader.blendAlphaFactorSrc = attachmentData.blendAlphaFactorSrc.value();
				attachmentHeader.blendAlphaOperation = attachmentData.blendAlphaOperation.value();
				attachmentHeader.blendColorFactorDst = attachmentData.blendColorFactorDst.value();
				attachmentHeader.blendColorFactorSrc = attachmentData.blendColorFactorSrc.value();
				attachmentHeader.blendColorOperation = attachmentData.blendColorOperation.value();
			}
		}
	}

}

static void OutputPipelineSetFile(
	std::vector<char>& outputBuffer,
	const std::vector<Grindstone::Formats::Pipelines::V1::MaterialParameter>& materialParameters,
	const std::vector<Grindstone::Formats::Pipelines::V1::MaterialResource>& materialResources,
	const std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineSetHeader>& graphicsPipelineSetHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineSetHeader>& computePipelineSetHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineConfigurationHeader>& graphicsConfigHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineConfigurationHeader>& computeConfigHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineHeader>& passHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader>& shaderStages,
	const std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader>& attachmentHeaders,
	const std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet>& descriptorSets,
	const std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding>& descriptorBindings,
	const std::vector<char>& blobs
) {
	// ========================================
	// Prepare Header
	// ========================================
	Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader pipelineSetFileHeader{};
	uint32_t totalSize = 4 + sizeof(Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader);
	{
		pipelineSetFileHeader.headerSize = sizeof(Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.graphicsPipelinesOffset, pipelineSetFileHeader.graphicsPipelineCount, graphicsPipelineSetHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.computePipelinesOffset, pipelineSetFileHeader.computePipelineCount, computePipelineSetHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.materialParametersOffset, pipelineSetFileHeader.materialParameterCount, materialParameters);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.materialResourcesOffset, pipelineSetFileHeader.materialResourceCount, materialResources);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.graphicsConfigurationsOffset, pipelineSetFileHeader.graphicsConfigurationCount, graphicsConfigHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.computeConfigurationsOffset, pipelineSetFileHeader.computeConfigurationCount, computeConfigHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.graphicsPassesOffset, pipelineSetFileHeader.graphicsPassCount, passHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.shaderStagesOffset, pipelineSetFileHeader.shaderStageCount, shaderStages);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.attachmentHeadersOffset, pipelineSetFileHeader.attachmentHeaderCount, attachmentHeaders);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.descriptorSetsOffset, pipelineSetFileHeader.descriptorSetCount, descriptorSets);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.descriptorBindingsOffset, pipelineSetFileHeader.descriptorBindingCount, descriptorBindings);
		AssignCountAndOffsetAndIncrementSize(totalSize, pipelineSetFileHeader.blobSectionOffset, pipelineSetFileHeader.blobSectionSize, blobs);
	}

	outputBuffer.resize(totalSize);

	// ========================================
	// Copy buffers to final buffer
	// ========================================
	Writer writer{ outputBuffer };
	WriteBytes(writer, Grindstone::Formats::Pipelines::V1::FileMagicCode, 4);
	WriteBytes(writer, &pipelineSetFileHeader, sizeof(Grindstone::Formats::Pipelines::V1::PipelineSetFileHeader));
	WriteBytes(writer, graphicsPipelineSetHeaders);
	WriteBytes(writer, computePipelineSetHeaders);
	WriteBytes(writer, materialParameters);
	WriteBytes(writer, materialResources);
	WriteBytes(writer, graphicsConfigHeaders);
	WriteBytes(writer, computeConfigHeaders);
	WriteBytes(writer, passHeaders);
	WriteBytes(writer, shaderStages);
	WriteBytes(writer, attachmentHeaders);
	WriteBytes(writer, descriptorSets);
	WriteBytes(writer, descriptorBindings);
	WriteBytes(writer, blobs);
}

bool OutputPipelineSet(LogCallback logCallback, const CompilationArtifactsGraphics& compilationArtifacts, const ResolvedStateTree::PipelineSet& pipelineSet, PipelineOutput& outputFile) {
	outputFile.name = pipelineSet.name;
	outputFile.pipelineType = Grindstone::Formats::Pipelines::V1::PipelineType::Graphics;
	std::vector<char>& outputBuffer = outputFile.content;

	std::vector<Grindstone::Formats::Pipelines::V1::MaterialParameter> materialParameters;
	std::vector<Grindstone::Formats::Pipelines::V1::MaterialResource> materialResources;
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineSetHeader> graphicsSetHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineSetHeader> computeSetHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineConfigurationHeader> graphicsConfigHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineConfigurationHeader> computeConfigHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineHeader> passHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader> shaderStages;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader> attachmentHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet> descriptorSets;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding> descriptorBindings;
	std::vector<char> blobs;
	Writer blobWriter{ blobs };

	ExtractGraphicsPipelineSet(
		logCallback,
		compilationArtifacts,
		pipelineSet,
		graphicsSetHeaders,
		graphicsConfigHeaders,
		passHeaders,
		shaderStages,
		attachmentHeaders,
		descriptorSets,
		descriptorBindings,
		materialParameters,
		materialResources,
		blobWriter
	);

	OutputPipelineSetFile(
		outputBuffer,
		materialParameters,
		materialResources,
		graphicsSetHeaders,
		computeSetHeaders,
		graphicsConfigHeaders,
		computeConfigHeaders,
		passHeaders,
		shaderStages,
		attachmentHeaders,
		descriptorSets,
		descriptorBindings,
		blobs
	);

	return true;
}

bool OutputComputeSet(LogCallback logCallback, const CompilationArtifactsCompute& compilationArtifacts, const ResolvedStateTree::ComputeSet& computeSet, PipelineOutput& outputFile) {
	logCallback(Grindstone::LogSeverity::Info, PipelineConverterLogSource::Output, "Compiled and Outputted", computeSet.sourceFilepath, UNDEFINED_LINE, UNDEFINED_COLUMN);
	outputFile.name = computeSet.name;
	outputFile.pipelineType = Grindstone::Formats::Pipelines::V1::PipelineType::Compute;
	outputFile.content.resize(compilationArtifacts.computeStage.compiledCode.size() + 4);

	std::vector<Grindstone::Formats::Pipelines::V1::MaterialParameter> materialParameters;
	std::vector<Grindstone::Formats::Pipelines::V1::MaterialResource> materialResources;
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineSetHeader> graphicsSetHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineSetHeader> computeSetHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::GraphicsPipelineConfigurationHeader> graphicsConfigHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ComputePipelineConfigurationHeader> computeConfigHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineHeader> passHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader> shaderStages;
	std::vector<Grindstone::Formats::Pipelines::V1::PassPipelineAttachmentHeader> attachmentHeaders;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet> descriptorSets;
	std::vector<Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding> descriptorBindings;
	std::vector<char> blobs;
	Writer blobWriter{ blobs };

	if (computeSet.shaderEntrypoint.empty()) {
		return false;
	}

	std::vector<DescriptorSetOutput> passDescriptorSets;
	const StageCompilationArtifacts& artifacts = compilationArtifacts.computeStage;
		
	Grindstone::GraphicsAPI::ShaderStageBit stageBit =
		static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(1 << static_cast<uint8_t>(artifacts.stage));

	ConsolidateDescriptorSets(
		stageBit,
		passDescriptorSets,
		artifacts.reflectedDescriptorSets,
		artifacts.reflectedDescriptorBindings
	);

	uint16_t descriptorSetOffset = static_cast<uint16_t>(descriptorSets.size());
	uint16_t descriptorBindingOffset = static_cast<uint16_t>(descriptorBindings.size());

	for (DescriptorSetOutput& srcSet : passDescriptorSets) {
		size_t firstIndex = descriptorBindings.size();
		size_t bindingCount = srcSet.bindings.size();

		for (Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorBinding& srcBinding : srcSet.bindings) {
			descriptorBindings.emplace_back(srcBinding);
		}

		Grindstone::Formats::Pipelines::V1::ShaderReflectDescriptorSet& dstSet = descriptorSets.emplace_back();
		dstSet.setIndex = srcSet.setIndex;
		dstSet.bindingStartIndex = static_cast<uint32_t>(firstIndex);
		dstSet.bindingCount = static_cast<uint32_t>(bindingCount);
	}

	Grindstone::Formats::Pipelines::V1::ComputePipelineSetHeader& computeHeader = computeSetHeaders.emplace_back();
	computeHeader = {};
	computeHeader.configurationStartIndex = static_cast<uint32_t>(computeConfigHeaders.size());
	computeHeader.configurationCount = 1;

	Grindstone::Formats::Pipelines::V1::ComputePipelineConfigurationHeader& computeConfigurationHeader = computeConfigHeaders.emplace_back();
	computeConfigurationHeader = {};
	computeConfigurationHeader.shaderStageIndex = static_cast<uint16_t>(shaderStages.size());
	computeConfigurationHeader.descriptorSetStartIndex = static_cast<uint16_t>(descriptorSetOffset);
	computeConfigurationHeader.descriptorSetCount = static_cast<uint8_t>(descriptorSets.size() - descriptorSetOffset);
	computeConfigurationHeader.descriptorBindingStartIndex = static_cast<uint16_t>(descriptorBindingOffset);
	computeConfigurationHeader.descriptorBindingCount = static_cast<uint8_t>(descriptorBindings.size() - descriptorBindingOffset);

	Grindstone::Formats::Pipelines::V1::PassPipelineShaderStageHeader& stageHeader = shaderStages.emplace_back();
	stageHeader = {};
	stageHeader.shaderCodeSize = static_cast<uint32_t>(artifacts.compiledCode.size());
	stageHeader.stageType = static_cast<Grindstone::GraphicsAPI::ShaderStage>(artifacts.stage);
	stageHeader.shaderCodeOffsetFromBlobStart = static_cast<uint32_t>(blobWriter.offset);
	WriteBytes(blobWriter, artifacts.compiledCode.data(), stageHeader.shaderCodeSize);

	OutputPipelineSetFile(
		outputFile.content,
		materialParameters,
		materialResources,
		graphicsSetHeaders,
		computeSetHeaders,
		graphicsConfigHeaders,
		computeConfigHeaders,
		passHeaders,
		shaderStages,
		attachmentHeaders,
		descriptorSets,
		descriptorBindings,
		blobs
	);

	return true;
}
