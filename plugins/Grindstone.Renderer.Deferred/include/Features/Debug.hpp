#pragma once

#include <Common/Rendering/RendererFeature.hpp>
#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::GraphicsAPI {
	class Buffer;
}

namespace Grindstone::Renderer {
	enum class DeferredRenderMode : uint16_t {
		Default,
		Position,
		PositionMod,
		ViewPosition,
		ViewPositionMod,
		Depth,
		DepthMod,
		Normal,
		ViewNormal,
		Albedo,
		Specular,
		Roughness,
		AmbientOcclusion,
		Count
	};

	class Debug : public Rendering::RendererFeature {
	public:
		Debug() : Rendering::RendererFeature("Debug", Rendering::DeferredRenderGraphOrderEvent::EndRendering) {}
		static const char* GetStaticFeatureName() { return "Debug"; }
		virtual void Initialize() override;
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) override;

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> debugPipelineSet;
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> debugDataDescriptorSet{};
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> debugDataUniformBuffer{};
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
