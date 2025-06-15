#pragma once

#include <string>
#include <vector>

#include <Common/Buffer.hpp>
#include <Common/HashedString.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <EngineCore/Assets/Asset.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <EngineCore/EngineCore.hpp>

#include "PipelineAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsPipeline;
		class DescriptorSetLayout;
	}

	struct GraphicsPipelineAsset : public Asset {
		constexpr static size_t noMaterialBufferIndex = SIZE_MAX;

		GraphicsPipelineAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		
		// TODO: Support Configurations
		struct Pass {
			std::string passPipelineName;
			Grindstone::HashedString renderQueue;
			GraphicsAPI::GraphicsPipeline::PipelineData pipelineData;
			std::array<GraphicsAPI::DescriptorSetLayout*, 16> descriptorSetLayouts;
			std::array<Grindstone::Buffer, GraphicsAPI::numShaderGraphicStage> stageBuffers;
			std::array<GraphicsAPI::ShaderStage, GraphicsAPI::numShaderGraphicStage> stageTypes;
			std::array<GraphicsAPI::GraphicsPipeline::AttachmentData, 8> colorAttachmentData;
		};

		struct MetaData {
			std::vector<Grindstone::PipelineAssetMetaData::Buffer> buffers;

			// Images and Samplers
			std::vector<Grindstone::PipelineAssetMetaData::ResourceSlot> resources;

			size_t materialBufferIndex = SIZE_MAX;
		};

		MetaData metaData;
		std::vector<Pass> passes;

		const Grindstone::PipelineAssetMetaData::Buffer* GetBufferMetaData() const {
			if (metaData.materialBufferIndex == SIZE_MAX) {
				return nullptr;
			}

			return &(metaData.buffers[metaData.materialBufferIndex]);
		}

		const Grindstone::PipelineAssetMetaData::ResourceSlot& GetTextureMetaDataByIndex(size_t index) const {
			return metaData.resources[index];
		}

		size_t GetTextureMetaDataSize() const {
			return metaData.resources.size();
		}

		Grindstone::GraphicsAPI::DescriptorSetLayout* GetMaterialDescriptorLayout() {
			if (passes.size() == 0) {
				return nullptr;
			}

			return passes[0].descriptorSetLayouts[0];
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetFirstPass() const {
			if (passes.size() == 0) {
				return nullptr;
			}

			return &passes[0];
		}

		const Grindstone::GraphicsAPI::GraphicsPipeline::PipelineData* GetFirstPassPipelineData() const {
			if (passes.size() == 0) {
				return nullptr;
			}

			return &passes[0].pipelineData;
		}

		Grindstone::GraphicsAPI::GraphicsPipeline* GetFirstPassPipeline(const Grindstone::GraphicsAPI::VertexInputLayout* vertexInputLayout) {
			if (passes.size() == 0) {
				return nullptr;
			}

			std::vector<Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData> stages;
			stages.resize(passes[0].pipelineData.shaderStageCreateInfoCount);

			for (size_t stageIndex = 0; stageIndex < passes[0].pipelineData.shaderStageCreateInfoCount; ++stageIndex) {
				stages[stageIndex].content = reinterpret_cast<const char*>(passes[0].stageBuffers[stageIndex].Get());
				stages[stageIndex].size = static_cast<uint32_t>(passes[0].stageBuffers[stageIndex].GetCapacity());
				stages[stageIndex].type = passes[0].stageTypes[stageIndex];
			}

			passes[0].pipelineData.debugName = passes[0].passPipelineName.c_str();
			passes[0].pipelineData.colorAttachmentData = passes[0].colorAttachmentData.data();
			passes[0].pipelineData.descriptorSetLayouts = passes[0].descriptorSetLayouts.data();
			passes[0].pipelineData.shaderStageCreateInfos = stages.data();

			Grindstone::GraphicsAPI::Core* graphicsCore = Grindstone::EngineCore::GetInstance().GetGraphicsCore();
			return graphicsCore->GetOrCreateGraphicsPipelineFromCache(passes[0].pipelineData, vertexInputLayout);
		}

		const Grindstone::GraphicsAPI::GraphicsPipeline* GetPassPipeline(Grindstone::HashedString renderQueue, const Grindstone::GraphicsAPI::VertexInputLayout* vertexInputLayout) const {
			const Grindstone::GraphicsPipelineAsset::Pass* selectedPass = nullptr;
			for (const Grindstone::GraphicsPipelineAsset::Pass& pass : passes) {
				if (pass.renderQueue == renderQueue) {
					selectedPass = &pass;
					break;
				}
			}

			if (selectedPass == nullptr) {
				return nullptr;
			}

			std::vector<Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData> stages;
			stages.resize(selectedPass->pipelineData.shaderStageCreateInfoCount);

			for (size_t stageIndex = 0; stageIndex < selectedPass->pipelineData.shaderStageCreateInfoCount; ++stageIndex) {
				stages[stageIndex].content = reinterpret_cast<const char*>(selectedPass->stageBuffers[stageIndex].Get());
				stages[stageIndex].size = static_cast<uint32_t>(selectedPass->stageBuffers[stageIndex].GetCapacity());
				stages[stageIndex].type = selectedPass->stageTypes[stageIndex];
			}

			GraphicsAPI::GraphicsPipeline::PipelineData pipelineData = selectedPass->pipelineData;
			pipelineData.debugName = selectedPass->passPipelineName.c_str();
			pipelineData.colorAttachmentData = selectedPass->colorAttachmentData.data();
			pipelineData.descriptorSetLayouts = selectedPass->descriptorSetLayouts.data();
			pipelineData.shaderStageCreateInfos = stages.data();

			Grindstone::GraphicsAPI::Core* graphicsCore = Grindstone::EngineCore::GetInstance().GetGraphicsCore();
			return graphicsCore->GetOrCreateGraphicsPipelineFromCache(pipelineData, vertexInputLayout);
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetPass(Grindstone::HashedString renderQueue) const {
			for (const Grindstone::GraphicsPipelineAsset::Pass& pass : passes) {
				if (pass.renderQueue == renderQueue) {
					return &pass;
				}
			}

			return nullptr;
		}

		DEFINE_ASSET_TYPE("Graphics PipelineSet", AssetType::GraphicsPipelineSet)
	};
}
