#pragma once

#include <Common/Rendering/RenderGraphBuilder.hpp>
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
		GbufferData AddPass(glm::mat4& projectionMatrix, glm::mat4 viewMatrix, Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder);

	private:
	};
}
