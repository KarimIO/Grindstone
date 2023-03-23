#pragma once

#include <string>
#include <vector>
#include <map>

#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Assets/AssetManager.hpp"

namespace Grindstone {
	struct ShaderAsset;

	struct RenderQueueContainer {
		std::vector<Uuid> shaders;
	};

	class BaseAssetRenderer {
	public:
		void AddShaderToRenderQueue(Uuid shaderUuid, const char* renderQueue) {
			auto& shaders = renderQueues[renderQueue].shaders;
			if (std::find(shaders.begin(), shaders.end(), shaderUuid) == shaders.end()) {
				shaders.push_back(shaderUuid);
			}
		}
		void AddQueue(const char* name);
		void RenderQueue(const char* name);
	private:
		virtual void RenderQueue(RenderQueueContainer& renderQueueContainer) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	};
}
