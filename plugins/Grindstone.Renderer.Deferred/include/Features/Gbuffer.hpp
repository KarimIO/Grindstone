#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	struct GbufferData {
		RenderGraphBuilderResourceRef albedoRef;
		RenderGraphBuilderResourceRef normalRef;
		RenderGraphBuilderResourceRef specularRoughnessRef;
		RenderGraphBuilderResourceRef depthRef;
	};

	class Gbuffer : public Rendering::RendererFeature {
	public:
		Gbuffer() : Rendering::RendererFeature("Gbuffer", Rendering::DeferredRenderGraphOrderEvent::OpaqueLitGeometry) {}
		static const char* GetStaticFeatureName() { return "Gbuffer"; }
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	};
}
