#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	struct ShadowPassReturnData {
		Renderer::RenderGraphBuilderResourceRef shadowOutputRef;
	};


	class ShadowPass {
	public:
		bool Initialize();
		ShadowPassReturnData AddShadowPasses(
			Grindstone::Renderer::RenderGraphBuilder& renderGraph,
			Grindstone::WorldContextSet& worldContextSet
		);

	protected:
		void PrepareAtlas(uint32_t totalShadowMapCount);
		bool GetAtlasRenderArea(Grindstone::Math::IntRect2D& rect);
		uint32_t currentAtlasIndex = 0;
		uint32_t shadowResolution = 512;
		uint32_t maxAtlasCount = 0;
	};
}
