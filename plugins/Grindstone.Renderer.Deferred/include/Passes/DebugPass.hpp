#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

#include "GbufferPass.hpp"

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

	class DebugPass {
	public:
		bool Initialize();
		Grindstone::Renderer::RenderGraphBuilderResourceRef AddPass(
			DeferredRenderMode renderMode,
			const glm::mat4& projectionMatrix,
			GraphicsAPI::Buffer* vertexBuffer,
			GraphicsAPI::Buffer* indexBuffer,
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			const GbufferData& gbufferData,
			RenderGraphBuilderResourceRef ambientOcclusionRef,
			RenderGraphBuilderResourceRef outputRef
		);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> debugPipelineSet;
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> debugDataDescriptorSet{};
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> debugDataUniformBuffer{};
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::Sampler* screenSampler = nullptr;
	};
}
