#pragma once

#include <string>
#include <vector>
#include <map>

namespace Grindstone {
	struct ShaderAsset;

	struct RenderQueueContainer {
		std::vector<ShaderAsset*> shaders;
	};

	class BaseAssetRenderer {
	public:
		void AddShaderToRenderQueue(ShaderAsset* shader);
		void AddQueue(const char* name);
		void RenderQueue(const char* name);
	private:
		virtual void RenderQueue(RenderQueueContainer& renderQueueContainer) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	};
}
