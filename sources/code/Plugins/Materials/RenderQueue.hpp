#pragma once

#include <vector>
#include <map>
#include <string>
#include "Pipeline.hpp"

namespace Grindstone {
	namespace Renderer {
		struct RenderQueue {
			std::string name_;
			std::vector<Renderer::Pipeline> pipelines_;
			std::map<std::string, uint32_t> string_to_pipeline_index_;
		}; // class MaterialManager
	} // namespace Renderer
} // namespace Grindstone
