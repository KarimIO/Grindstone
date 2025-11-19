#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include "RenderGraph.hpp"

namespace Grindstone::Renderer {
	class RenderGraphBuilder {
	public:
		// Using all current conditions, evaluate all the framegraph passes and output them to a compiled frame graph with all resources prepared for the render.
		[[nodiscard]] RenderGraph Compile();
		RenderGraphPass& AddPass(RenderGraphPass pass);
		RenderGraphPass& AddPass(HashedString name, GpuQueue queue);

		void SetOutputAttachment(HashedString attachmentName);

	private:
		HashedString outputAttachment;
		std::map<HashedString, RenderGraphPass> passes;
		std::map<HashedString, Grindstone::GraphicsAPI::Buffer*> buffers;
		std::map<HashedString, Grindstone::GraphicsAPI::Image*> images;
	};
}
