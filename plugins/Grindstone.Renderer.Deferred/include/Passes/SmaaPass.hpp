#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::GraphicsAPI {
	class DescriptorSetLayout;
	class DescriptorSet;
	class Buffer;
	class Sampler;
}

namespace Grindstone::Renderer {
	class SmaaPass {
	public:
		bool Initialize();
		Grindstone::Renderer::RenderGraphBuilderResourceRef AddPass(
			Grindstone::Renderer::RenderGraphBuilder& renderGraph,
			Grindstone::Renderer::RenderGraphBuilderResourceRef lightingImageRef,
			Grindstone::Renderer::RenderGraphBuilderResourceRef outputImageRef
		);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> smaaPipelineSet;
		Grindstone::AssetReference<Grindstone::TextureAsset> smaaAreaTexture;
		Grindstone::AssetReference<Grindstone::TextureAsset> smaaSearchTexture;
		GraphicsAPI::DescriptorSetLayout* smaaDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* smaaDescriptorSet = nullptr;
		GraphicsAPI::Sampler* pointSampler = nullptr;
		GraphicsAPI::Sampler* linearSampler = nullptr;
	};
}
