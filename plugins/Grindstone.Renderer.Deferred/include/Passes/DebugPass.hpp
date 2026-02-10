#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class DebugPass {
	public:
		bool Initialize();
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> debugPipelineSet;
	};
}
