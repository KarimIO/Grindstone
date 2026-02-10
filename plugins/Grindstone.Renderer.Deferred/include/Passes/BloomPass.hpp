#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class BloomPass {
	public:
		bool Initialize();
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph);

	private:
		void CreateDescriptorSets();
		void UpdateBloomUBO();
		void CreateBloomResources();
		void UpdateBloomDescriptorSet();
		void CreateUniformBuffers();
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> bloomPipelineSet;
	};
}
