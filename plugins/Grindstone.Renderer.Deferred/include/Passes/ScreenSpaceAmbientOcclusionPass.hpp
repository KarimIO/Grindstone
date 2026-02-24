#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

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
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> ssaoPipelineSet;
		void CreateSsaoKernelAndNoise();

		Grindstone::GraphicsAPI::Buffer* ssaoUniformBuffer;
		Grindstone::GraphicsAPI::Image* ssaoNoiseTexture;
		Grindstone::GraphicsAPI::Sampler* ssaoNoiseSampler;
		Grindstone::GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout;
		Grindstone::GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet;
	};
}
