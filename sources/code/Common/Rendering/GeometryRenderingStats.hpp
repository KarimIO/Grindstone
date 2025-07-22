 #pragma once

 #include <Common/HashedString.hpp>

namespace Grindstone::Rendering {
	struct GeometryRenderStats {
		Grindstone::HashedString renderQueue;

		uint32_t drawCalls = 0;
		uint32_t triangles = 0;
		uint32_t vertices = 0;
		uint32_t objectsCulled = 0;
		uint32_t objectsRendered = 0;
		uint32_t pipelineBinds = 0;
		uint32_t materialBinds = 0;

		double gpuTimeMs = 0.0;
		double cpuTimeMs = 0.0;
	}; // struct RenderStats
} // namespace Grindstone::Rendering
