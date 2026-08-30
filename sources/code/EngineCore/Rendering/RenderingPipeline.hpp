#pragma once

#include <map>
#include <glm/glm.hpp>

#include <Blackboard/Blackboard.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <Common/Rendering/RendererFeature.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
	}

	namespace Renderer {
		struct RenderMode {
			const char* name = nullptr;
		};

		class RenderingPipeline {
		public:
			virtual void Render(
				Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
				Grindstone::Renderer::RenderFrameContext& renderFrameContext
			);

			virtual std::vector<Grindstone::Rendering::GeometryRenderStats> GetRenderingStats();
			virtual void PushRenderingStats(const Grindstone::Rendering::GeometryRenderStats& stats);

			virtual void RegisterFeature(Grindstone::Rendering::RendererFeature* feature);
			virtual void UnregisterFeature(const char* name);
		
		protected:
			std::vector<Grindstone::Rendering::GeometryRenderStats> renderStats;
			std::vector<Grindstone::Rendering::RendererFeature*> features;
		
		};
	}
}
