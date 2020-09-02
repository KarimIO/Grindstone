#pragma once

#include <vector>
#include <stdint.h>

#include "RenderQueue.hpp"

namespace Grindstone {
	namespace Renderer {
		class MaterialManager {
		public:
			// Render Groups
			virtual uint32_t addQueue(const char* name);

			// Materials
			virtual void loadMaterial(const char* name);
			virtual void loadMaterialImmediate(const char* name);
			virtual void reloadMaterial(const char* name);
			virtual void removeMaterial(const char* name);

			// Pipelines
			virtual void loadPipeline(const char* name);
			virtual void loadPipelineImmediate(const char* name);
			virtual void reloadPipeline(const char* name);
			virtual void removePipeline(const char* name);

			std::vector<RenderQueue> render_queues_;
		private:
			void loadMaterialImpl(const char* name);
			void loadPipelineImpl(const char* name);

		}; // class MaterialManager
	} // namespace Renderer
} // namespace Grindstone
