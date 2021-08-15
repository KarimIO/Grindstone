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
		void AddShaderToRenderQueue(Shader* shader);
		void AddQueue(const char* name);
		void RenderQueue(const char* name);
		Material* GetErrorMaterial();
	private:
		virtual void RenderQueue(RenderQueueContainer& renderQueueContainer) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	protected:
		Material* errorMaterial;
	};
}
