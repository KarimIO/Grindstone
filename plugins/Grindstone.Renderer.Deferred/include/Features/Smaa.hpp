#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::GraphicsAPI {
	class DescriptorSetLayout;
	class DescriptorSet;
	class Buffer;
	class Sampler;
}

namespace Grindstone::Renderer {
	class Smaa : public Rendering::RendererFeature {
	public:
		Smaa() : Rendering::RendererFeature("Smaa", Rendering::DeferredRenderGraphOrderEvent::AntiAliasingLater) {}
		static const char* GetStaticFeatureName() { return "Smaa"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> smaaPipelineSet;
		Grindstone::AssetReference<Grindstone::TextureAsset> smaaAreaTexture;
		Grindstone::AssetReference<Grindstone::TextureAsset> smaaSearchTexture;
		GraphicsAPI::DescriptorSetLayout* smaaDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* smaaDescriptorSet = nullptr;
		GraphicsAPI::Sampler* pointSampler = nullptr;
		GraphicsAPI::Sampler* linearSampler = nullptr;
	};
}
