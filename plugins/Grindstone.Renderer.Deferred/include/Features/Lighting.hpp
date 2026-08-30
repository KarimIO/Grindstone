#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class Lighting : public Rendering::RendererFeature {
	public:
		Lighting() : Rendering::RendererFeature("Lighting", Rendering::DeferredRenderGraphOrderEvent::Lighting) {}
		static const char* GetStaticFeatureName() { return "Lighting"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> imageBasedLightingPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> spotLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pointLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> directionalLightPipelineSet;
		Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut;

		Grindstone::GraphicsAPI::Image* currentEnvironmentMapImage = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* ambientOcclusionDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* ambientOcclusionDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
