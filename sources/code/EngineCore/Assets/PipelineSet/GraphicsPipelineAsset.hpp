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
			std::string passDebugName;
			std::string passPipelineName;
			Grindstone::HashedString renderQueue;
			GraphicsAPI::PipelineLayout* pipelineLayout = nullptr;
			GraphicsAPI::GraphicsPipeline::PipelineData pipelineData;
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

		const Grindstone::GraphicsAPI::DescriptorSetLayout* GetMaterialDescriptorLayout() const {
			if (passes.size() == 0) {
				return nullptr;
			}

			return passes[0].pipelineLayout->descriptorSetLayouts[1];
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

			auto& pass = passes[0];
			return GetPassPipeline(pass, vertexInputLayout);
		}

		Grindstone::GraphicsAPI::PipelineLayout* GetFirstPassPipelineLayout() {
			if (passes.size() == 0) {
				return nullptr;
			}

			auto& pass = passes[0];
			return pass.pipelineLayout;
		}

		Grindstone::GraphicsAPI::PipelineLayout* GetPassPipelineLayout(Grindstone::HashedString renderQueue) {
			const Grindstone::GraphicsPipelineAsset::Pass* selectedPass = GetPassByRenderQueue(renderQueue);

			if (selectedPass == nullptr) {
				return nullptr;
			}

			auto& pass = passes[0];
			return pass.pipelineLayout;
		}

		Grindstone::GraphicsAPI::GraphicsPipeline* GetPassPipelineByRenderQueue(Grindstone::HashedString renderQueue, const Grindstone::GraphicsAPI::VertexInputLayout* vertexInputLayout) const {
			const Grindstone::GraphicsPipelineAsset::Pass* selectedPass = GetPassByRenderQueue(renderQueue);

			if (selectedPass == nullptr) {
				return nullptr;
			}

			return GetPassPipeline(*selectedPass, vertexInputLayout);
		}

		Grindstone::GraphicsAPI::GraphicsPipeline* GetPassPipelineByName(std::string name, const Grindstone::GraphicsAPI::VertexInputLayout* vertexInputLayout) const {
			const Grindstone::GraphicsPipelineAsset::Pass* selectedPass = GetPassByName(name);

			if (selectedPass == nullptr) {
				return nullptr;
			}

			return GetPassPipeline(*selectedPass, vertexInputLayout);
		}

		Grindstone::GraphicsAPI::GraphicsPipeline* GetPassPipeline(const Grindstone::GraphicsPipelineAsset::Pass& pass, const Grindstone::GraphicsAPI::VertexInputLayout* vertexInputLayout) const {
			std::vector<Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData> stages;
			stages.resize(pass.pipelineData.shaderStageCreateInfoCount);

			for (size_t stageIndex = 0; stageIndex < pass.pipelineData.shaderStageCreateInfoCount; ++stageIndex) {
				stages[stageIndex].content = reinterpret_cast<const char*>(pass.stageBuffers[stageIndex].Get());
				stages[stageIndex].size = static_cast<uint32_t>(pass.stageBuffers[stageIndex].GetCapacity());
				stages[stageIndex].type = pass.stageTypes[stageIndex];
			}

			GraphicsAPI::PipelineLayout* pipelineLayout = pass.pipelineLayout;
			GraphicsAPI::GraphicsPipeline::PipelineData pipelineData = pass.pipelineData;
			pipelineData.debugName = pass.passDebugName.c_str();
			pipelineData.colorAttachmentData = pass.colorAttachmentData.data();
			pipelineData.shaderStageCreateInfos = stages.data();

			Grindstone::GraphicsAPI::Core* graphicsCore = Grindstone::EngineCore::GetInstance().GetGraphicsCore();
			return graphicsCore->GetOrCreateGraphicsPipelineFromCache(pipelineLayout, pipelineData, vertexInputLayout);
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetPassByName(std::string name) const {
			for (const Grindstone::GraphicsPipelineAsset::Pass& pass : passes) {
				if (pass.passPipelineName == name) {
					return &pass;
				}
			}

			return nullptr;
		}

		const Grindstone::GraphicsPipelineAsset::Pass* GetPassByRenderQueue(Grindstone::HashedString renderQueue) const {
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
