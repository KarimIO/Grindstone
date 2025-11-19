#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include "RenderGraphPass.hpp"
#include "GpuQueue.hpp"

namespace Grindstone::GraphicsAPI {
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	class RenderGraph {
	public:
		void Print();

		// Using all current conditions, evaluate all the framegraph passes and output them to a compiled frame graph with all resources prepared for the render.
		RenderGraphPass& AddPass(RenderGraphPass pass);
		RenderGraphPass& AddPass(HashedString name, GpuQueue queue);

	private:
		std::map<HashedString, RenderGraphPass> passes;
		std::map<HashedString, Grindstone::GraphicsAPI::Buffer*> buffers;
		std::map<HashedString, Grindstone::GraphicsAPI::Image*> images;
	};
}
