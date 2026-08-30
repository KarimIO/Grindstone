#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Editor::RendererFeatures {
	class Grid : public Rendering::RendererFeature {
	public:
		Grid() : Rendering::RendererFeature("Grid", Rendering::DeferredRenderGraphOrderEvent::EndRendering - 2) {}
		static const char* GetStaticFeatureName() { return "Grid"; }
		virtual void Initialize() override;
		virtual void Bind(Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pipelineSet;
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> gridUniformBuffers = {};
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> gridDescriptorSets = {};
		Grindstone::GraphicsAPI::DescriptorSetLayout* gridDescriptorSetLayout = nullptr;
	};
}
