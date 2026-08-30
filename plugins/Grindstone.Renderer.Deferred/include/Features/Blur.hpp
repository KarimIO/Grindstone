#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class Blur : public Rendering::RendererFeature {
	public:
		Blur() : Rendering::RendererFeature("Blur", Rendering::DeferredRenderGraphOrderEvent::ScreenSpaceAmbientOcclusion + 1) {}
		static const char* GetStaticFeatureName() { return "Blur"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> blurPipelineSet;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
