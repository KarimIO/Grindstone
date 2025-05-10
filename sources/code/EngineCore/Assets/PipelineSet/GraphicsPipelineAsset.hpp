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
		GraphicsPipelineAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		
		// TODO: Support Configurations
		struct Pass {
			std::string passPipelineName;
			uint8_t renderQueue = UINT8_MAX;
			Grindstone::HashedString renderPass;
			Grindstone::HashedString orderBucket;
			GraphicsAPI::GraphicsPipeline::PipelineData pipelineData;
			std::array<GraphicsAPI::DescriptorSetLayout*, 16> descriptorSetLayouts;
			std::array<Grindstone::Buffer, GraphicsAPI::numShaderGraphicStage> stageBuffers;
			std::array<GraphicsAPI::ShaderStage, GraphicsAPI::numShaderGraphicStage> stageTypes;
			std::array<GraphicsAPI::GraphicsPipeline::AttachmentData, 8> colorAttachmentData;

			std::vector<size_t> bufferMetaDataIndices;
			std::vector<size_t> textureMetaDataIndices;
		};

		struct MetaData {
			std::vector<Grindstone::PipelineAssetMetaData::Buffer> buffers;
			std::vector<Grindstone::PipelineAssetMetaData::TextureSlot> textures;

			size_t materialBufferIndex = SIZE_MAX;
			std::vector<size_t> materialTextureMetaDataIndices;
		};

		MetaData metaData;
		std::vector<Pass> passes;

		const Grindstone::PipelineAssetMetaData::Buffer* GetBufferMetaData() const {
			if (metaData.materialBufferIndex == SIZE_MAX) {
				return nullptr;
			}

			return &(metaData.buffers[metaData.materialBufferIndex]);
		}

		const Grindstone::PipelineAssetMetaData::TextureSlot& GetTextureMetaDataByIndex(size_t index) const {
			return metaData.textures[metaData.materialTextureMetaDataIndices[index]];
		}

		size_t GetTextureMetaDataSize() const {
			return metaData.materialTextureMetaDataIndices.size();
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

		Grindstone::GraphicsPipelineAsset::Pass* GetFirstPass() {
			if (passes.size() == 0) {
				return nullptr;
			}

			return &passes[0];
		}

		Grindstone::GraphicsAPI::GraphicsPipeline::PipelineData* GetFirstPassPipelineData() {
			if (passes.size() == 0) {
				return nullptr;
			}

			return &passes[0].pipelineData;
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetPass(Grindstone::HashedString renderPass) const {
			for (const Grindstone::GraphicsPipelineAsset::Pass& pass : passes) {
				if (pass.renderPass == renderPass) {
					return &pass;
				}
			}

			return nullptr;
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetPass(Grindstone::HashedString renderPass, Grindstone::HashedString orderBucket) const {
			for (const Grindstone::GraphicsPipelineAsset::Pass& pass : passes) {
				if (pass.renderPass == renderPass && pass.orderBucket == orderBucket) {
					return &pass;
				}
			}

			return nullptr;
		}

		DEFINE_ASSET_TYPE("Graphics PipelineSet", AssetType::GraphicsPipelineSet)
	};
}
