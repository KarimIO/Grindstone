#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class DepthOfField : public Rendering::RendererFeature {
	public:
		DepthOfField() : Rendering::RendererFeature("DepthOfField", Rendering::DeferredRenderGraphOrderEvent::DepthOfField) {}
		static const char* GetStaticFeatureName() { return "DepthOfField"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofSeparationPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofBlurPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofCombinationPipelineSet;
	};
}
