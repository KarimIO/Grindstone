#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

#include "GbufferPass.hpp"

namespace Grindstone::Renderer {
	class ScreenSpaceReflectionsPass {
	public:
		bool Initialize();
		Renderer::RenderGraphBuilderResourceRef AddPass(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Renderer::RenderGraphBuilderResourceRef inputRef,
			Renderer::RenderGraphBuilderResourceRef ambientOcclusionRef,
			Grindstone::Renderer::GbufferData& gbufferData
		);

	private:
		bool FindEnvironmentMap(
			const entt::registry& registry,
			const Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut,
			Grindstone::GraphicsAPI::DescriptorSet* reflectionDescriptorSet,
			Grindstone::GraphicsAPI::Image*& currentEnvironmentMapImage
		);

		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> ssrPipelineSet;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;

		Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut;
		Grindstone::GraphicsAPI::Image* currentEnvironmentMapImage = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* reflectionDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* reflectionDescriptorSet = nullptr;
	};
}
