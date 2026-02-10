#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class ScreenSpaceReflectionsPass {
	public:
		bool Initialize();
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph);

	private:
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> ssrPipelineSet;
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> descriptorSets;
		Grindstone::GraphicsAPI::DescriptorSetLayout* ssrDescriptorSetLayout = nullptr;
	};
}
