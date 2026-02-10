#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class DepthOfFieldPass {
	public:
		bool Initialize();
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofSeparationPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofBlurPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofCombinationPipelineSet;
	};
}
