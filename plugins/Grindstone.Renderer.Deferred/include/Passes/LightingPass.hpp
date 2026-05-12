#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

#include "GbufferPass.hpp"

namespace Grindstone::Renderer {
	struct LightingPassReturnData {
		RenderGraphBuilderResourceRef lightingOutputRef;
	};

	class LightingPass {
	public:
		bool Initialize();
		LightingPassReturnData AddPass(
			GraphicsAPI::Buffer* vertexBuffer,
			GraphicsAPI::Buffer* indexBuffer,
			Grindstone::Renderer::RenderGraphBuilder& renderGraph,
			Grindstone::Renderer::GbufferData& gbufferData,
			RenderGraphBuilderResourceRef shadowAtlasRef
		);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> imageBasedLightingPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> spotLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pointLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> directionalLightPipelineSet;

		Grindstone::GraphicsAPI::Image* currentEnvironmentMapImage = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
