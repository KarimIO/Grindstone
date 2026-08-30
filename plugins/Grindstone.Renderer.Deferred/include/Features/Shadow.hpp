#pragma once

#include <functional>

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraph.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	struct ShadowPassReturnData {
		Renderer::RenderGraphBuilderResourceRef shadowOutputRef;
	};

	class Shadow : public Rendering::RendererFeature {
	public:
		Shadow() : Rendering::RendererFeature("Shadow", Rendering::DeferredRenderGraphOrderEvent::RenderingDirectionalShadows) {}
		static const char* GetStaticFeatureName() { return "Shadow"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	protected:
		void PrepareAtlas(uint32_t totalShadowMapCount);
		bool GetAtlasRenderArea(Grindstone::Math::IntRect2D& rect);
		uint32_t currentAtlasIndex = 0;
		uint32_t shadowResolution = 512;
		uint32_t maxAtlasCount = 0;
	};
}
