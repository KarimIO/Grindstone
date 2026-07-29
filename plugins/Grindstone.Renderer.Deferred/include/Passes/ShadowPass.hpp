#pragma once

#include <functional>

#include <Common/Rendering/RenderGraph.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Renderer {
	struct ShadowPassReturnData {
		Renderer::RenderGraphBuilderResourceRef shadowOutputRef;
	};


	class ShadowPass {
	public:
		bool Initialize();
		ShadowPassReturnData AddShadowPasses(
			const glm::vec3& eyePos,
			const glm::mat4& cameraProjectionMatrix,
			const glm::mat4& cameraViewMatrix,
			Grindstone::Renderer::RenderGraphBuilder& renderGraph,
			Grindstone::WorldContextSet& worldContextSet,
			std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
		);

	protected:
		void PrepareAtlas(uint32_t totalShadowMapCount);
		bool GetAtlasRenderArea(Grindstone::Math::IntRect2D& rect);
		uint32_t currentAtlasIndex = 0;
		uint32_t shadowResolution = 512;
		uint32_t maxAtlasCount = 0;
	};
}
