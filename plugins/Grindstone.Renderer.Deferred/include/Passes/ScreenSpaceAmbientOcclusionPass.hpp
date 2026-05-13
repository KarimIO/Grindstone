#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

#include "GbufferPass.hpp"

namespace Grindstone::GraphicsAPI {
	class Buffer;
	class Image;
	class Sampler;
	class DescriptorSet;
	class DescriptorSetLayout;
}
namespace Grindstone::Renderer {
	class ScreenSpaceAmbientOcclusionPass {
	public:
		bool Initialize();
		RenderGraphBuilderResourceRef AddPass(
			GraphicsAPI::Buffer* vertexBuffer,
			GraphicsAPI::Buffer* indexBuffer,
			Grindstone::Renderer::RenderGraphBuilder& renderGraph,
			const GbufferData& gbufferData
		);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> ssaoPipelineSet;
		void CreateSsaoKernelAndNoise();

		Grindstone::GraphicsAPI::Buffer* ssaoUniformBuffer;
		Grindstone::GraphicsAPI::Image* ssaoNoiseTexture;
		Grindstone::GraphicsAPI::Sampler* screenSampler;
		Grindstone::GraphicsAPI::Sampler* ssaoNoiseSampler;
		Grindstone::GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout;
		Grindstone::GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet;
	};
}
