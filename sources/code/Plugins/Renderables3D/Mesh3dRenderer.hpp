#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/mat4x4.hpp>

#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	struct RenderTask {
		GraphicsAPI::DescriptorSet* materialDescriptorSet;
		GraphicsAPI::DescriptorSet* perDrawDescriptorSet;
		class GraphicsAPI::GraphicsPipeline* pipeline;
		class GraphicsAPI::VertexArrayObject* vertexArrayObject;
		uint32_t indexCount;
		uint32_t baseVertex;
		uint32_t baseIndex;
		glm::mat4 transformMatrix;
		uint32_t sortData;
	};

	using RenderTaskList = std::vector<RenderTask>;
	using SortedRender = std::vector<size_t>;

	struct RenderQueueContainer {
		DrawSortMode sortMode;
		RenderTaskList renderTasks;

		RenderQueueContainer() = default;
		RenderQueueContainer(DrawSortMode drawSortMode) {
			sortMode = drawSortMode;
		}
	};

	class EngineCore;

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer(EngineCore* engineCore);

			virtual void AddQueue(const char* queueName, DrawSortMode sortMode);
			virtual void RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet) override;
			virtual void RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* queueName) override;
			virtual void CacheRenderTasksAndFrustumCull(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry) override;
		private:
			virtual std::string GetName() const override;
			virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) override;

			EngineCore* engineCore = nullptr;
			std::string rendererName = "Mesh3d";
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

			std::map<std::string, RenderQueueIndex> renderQueueMap;
			std::vector<RenderQueueContainer> renderQueues;
	};
}
