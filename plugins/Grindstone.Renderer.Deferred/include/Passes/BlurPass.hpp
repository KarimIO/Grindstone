#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class BlurPass {
	public:
		bool Initialize();
		Renderer::RenderGraphBuilderResourceRef AddPass(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			MetaRect& metaRect,
			Renderer::ImageDescription& imageDescription,
			Renderer::RenderGraphBuilderResourceRef imageToBlurRef
		);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> blurPipelineSet;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
