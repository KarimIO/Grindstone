#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class BloomPass {
	public:
		bool Initialize();
		Renderer::RenderGraphBuilderResourceRef AddBloomChain(
			uint32_t imageIndex,
			Grindstone::Math::Uint2 size,
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Renderer::RenderGraphBuilderResourceRef input
		);

		static const size_t BLOOM_MIPS = 4u;
		static const size_t BLOOM_STAGE_COUNT = BLOOM_MIPS * 2u;

		struct ImageSet {
			std::array<Grindstone::GraphicsAPI::DescriptorSet*, BLOOM_STAGE_COUNT> descriptorSets{};
			std::array<Grindstone::GraphicsAPI::Buffer*, BLOOM_STAGE_COUNT> bloomStageBuffers{};
			Grindstone::GraphicsAPI::Buffer* bloomDataBuffer{};
			Grindstone::Math::Uint2 size;
		};

	private:
		void CreateDescriptorSets();
		void UpdateBloomUBO();
		void CreateBloomResources();
		void UpdateBloomDescriptorSet();
		void CreateUniformBuffers();
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> bloomPipelineSet;
		Grindstone::GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;

		std::array<ImageSet, 3> imageSets;
	};
}
