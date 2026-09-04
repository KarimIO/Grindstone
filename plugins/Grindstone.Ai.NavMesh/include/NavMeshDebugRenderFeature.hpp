#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <Editor/GizmoRenderer.hpp>

namespace Grindstone::Ai::RendererFeatures {
	class NavMeshDebug : public Rendering::RendererFeature {
	public:
		NavMeshDebug() : Rendering::RendererFeature("NavMeshDebug", Rendering::DeferredRenderGraphOrderEvent::EndRendering - 1) {}
		static const char* GetStaticFeatureName() { return "NavMeshDebug"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> navmeshPipelineSet;
		Grindstone::GraphicsAPI::DescriptorSet* navmeshDescriptorSet = nullptr;
	};
}
