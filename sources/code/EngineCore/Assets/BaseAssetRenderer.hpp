#pragma once

#include <string>
#include <vector>
#include <map>

#include "Shaders/Shader.hpp"

namespace Grindstone {
	struct RenderQueueContainer {
		std::vector<Shader*> shaders;
	};

	class BaseAssetRenderer {
	public:
		void AddQueue(const char* name, RenderQueueContainer renderQueueContainer);
		void RenderQueue(const char* name);
	private:
		virtual void RenderQueue(RenderQueueContainer& renderQueueContainer) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	};
}
