#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class ScreenSpaceReflections : public Rendering::RendererFeature {
	public:
		ScreenSpaceReflections() : Rendering::RendererFeature("ScreenSpaceReflections", Rendering::DeferredRenderGraphOrderEvent::SpecularSSR) {}
		static const char* GetStaticFeatureName() { return "ScreenSpaceReflections"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

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
