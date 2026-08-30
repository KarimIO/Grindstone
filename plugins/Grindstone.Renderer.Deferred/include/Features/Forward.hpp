#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	class Forward : public Rendering::RendererFeature {
	public:
		Forward() : Rendering::RendererFeature("Forward", Rendering::DeferredRenderGraphOrderEvent::Transparent) {}
		static const char* GetStaticFeatureName() { return "Forward"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	};
}
