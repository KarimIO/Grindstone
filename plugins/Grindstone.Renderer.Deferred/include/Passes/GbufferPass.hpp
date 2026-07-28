#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	struct GbufferData {
		RenderGraphBuilderResourceRef albedoRef;
		RenderGraphBuilderResourceRef normalRef;
		RenderGraphBuilderResourceRef specularRoughnessRef;
		RenderGraphBuilderResourceRef depthRef;
	};

	class GbufferPass {
	public:
		bool Initialize();
		GbufferData AddPass(
			RenderGraphBuilderResourceRef depthImageRef,
			glm::mat4& projectionMatrix,
			glm::mat4 viewMatrix,
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
		);

	private:
	};
}
