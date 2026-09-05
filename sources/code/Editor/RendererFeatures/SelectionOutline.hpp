#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Editor::RendererFeatures {
	class SelectionOutline : public Rendering::RendererFeature {
	public:
		SelectionOutline() : Rendering::RendererFeature("SelectionOutline", Rendering::DeferredRenderGraphOrderEvent::EndRendering - 3) {}
		static const char* GetStaticFeatureName() { return "SelectionOutline"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;
	protected:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> outlinePipelineSet;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
