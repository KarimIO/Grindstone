#pragma once

#include <string>
#include <vector>
#include <map>

#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Assets/AssetManager.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

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
		virtual void RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet) = 0;
		void RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* name);
		void RenderQueueImmediate(const char* name);

		virtual std::string& GetName() = 0;
		virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) = 0;
	protected:
		virtual void RenderQueueImmediate(RenderQueueContainer& renderQueue) = 0;
		virtual void RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, RenderQueueContainer& renderQueue) = 0;
		std::map<std::string, RenderQueueContainer> renderQueues;
	};
}
