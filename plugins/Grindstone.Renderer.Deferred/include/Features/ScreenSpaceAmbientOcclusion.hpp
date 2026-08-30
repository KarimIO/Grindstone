#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::GraphicsAPI {
	class Buffer;
	class Image;
	class Sampler;
	class DescriptorSet;
	class DescriptorSetLayout;
}
namespace Grindstone::Renderer {
	class ScreenSpaceAmbientOcclusion : public Rendering::RendererFeature {
	public:
		ScreenSpaceAmbientOcclusion() : Rendering::RendererFeature("ScreenSpaceAmbientOcclusion", Rendering::DeferredRenderGraphOrderEvent::ScreenSpaceAmbientOcclusion) {}
		static const char* GetStaticFeatureName() { return "ScreenSpaceAmbientOcclusion"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> ssaoPipelineSet;
		void CreateSsaoKernelAndNoise();

		Grindstone::GraphicsAPI::Buffer* ssaoUniformBuffer = nullptr;
		Grindstone::GraphicsAPI::Image* ssaoNoiseTexture = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
		Grindstone::GraphicsAPI::Sampler* ssaoNoiseSampler = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet = nullptr;
	};
}
