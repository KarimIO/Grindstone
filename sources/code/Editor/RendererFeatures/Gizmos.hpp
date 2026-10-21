#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <Editor/GizmoRenderer.hpp>

namespace Grindstone::Editor::RendererFeatures {
	class Gizmos : public Rendering::RendererFeature {
	public:
		Gizmos() : Rendering::RendererFeature("Gizmos", Rendering::DeferredRenderGraphOrderEvent::EndRendering - 1) {}
		static const char* GetStaticFeatureName() { return "Gizmos"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;
		virtual ~Gizmos() override;

	private:
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
