#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>

namespace Grindstone::Renderer {
	class Skinning : public Rendering::RendererFeature {
	public:
		Skinning() : Rendering::RendererFeature("Skinning", Rendering::DeferredRenderGraphOrderEvent::Skinning) {}
		static const char* GetStaticFeatureName() { return "Skinning"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> skinningPipelineSet;
		Grindstone::GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

	};
}
