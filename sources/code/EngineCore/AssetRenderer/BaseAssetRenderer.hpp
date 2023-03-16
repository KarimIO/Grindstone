#pragma once

#include <string>
#include <vector>
#include <map>

#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"

namespace Grindstone {
	struct ShaderAsset;

	struct RenderQueueContainer {
		std::vector<ShaderAsset*> shaders;
	};

	class BaseAssetRenderer {
	public:
		void AddShaderToRenderQueue(ShaderAsset* shader) {
			const char* renderQueue = shader->reflectionData.renderQueue.c_str();
			renderQueues[renderQueue].shaders.push_back(shader);
		}
		void AddQueue(const char* name);
		void RenderQueue(const char* name);
	private:
		virtual void RenderQueue(RenderQueueContainer& renderQueueContainer) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	};
}
