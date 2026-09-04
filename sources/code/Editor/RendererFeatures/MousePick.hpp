#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Editor::RendererFeatures {
	class MousePick : public Rendering::RendererFeature {
	public:
		MousePick() : Rendering::RendererFeature("MousePick", Rendering::DeferredRenderGraphOrderEvent::EndRendering - 3) {}
		static const char* GetStaticFeatureName() { return "MousePick"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

		static uint32_t GetMousePickedEntity(GraphicsAPI::CommandBuffer* commandBuffer);

	private:
		GraphicsAPI::DescriptorSetLayout* mousePickDescriptorSetLayout = nullptr;

		std::array<GraphicsAPI::Image*, 3> mousePickRenderTarget{};
		std::array<GraphicsAPI::Framebuffer*, 3> mousePickFramebuffer{};
		std::array<GraphicsAPI::DescriptorSet*, 3> mousePickDescriptorSet{};
		std::array<GraphicsAPI::Buffer*, 3> mousePickMatrixBuffer{};
		static std::array<GraphicsAPI::Buffer*, 3> mousePickResponseBuffer;
	};
}
