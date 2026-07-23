#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class SkinningPass {
	public:
		bool Initialize();
		void AddPass(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::WorldContextSet& worldContextSet
		);

	private:
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> skinningPipelineSet;
		Grindstone::GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

	};
}
