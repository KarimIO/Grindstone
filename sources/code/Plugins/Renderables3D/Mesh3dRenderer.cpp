#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};

void Mesh3dRenderer::AddQueue(const char* queueName, DrawSortMode drawSortMode) {
	RenderQueueIndex index = static_cast<RenderQueueIndex>(renderQueues.size());
	RenderQueueContainer& queue = renderQueues.emplace_back(drawSortMode);
	renderQueueMap[queueName] = index;
}

Grindstone::Mesh3dRenderer::Mesh3dRenderer(EngineCore* engineCore) {
	this->engineCore = engineCore;
}

void Mesh3dRenderer::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	engineDescriptorSet = descriptorSet;
}

std::string Mesh3dRenderer::GetName() const {
	return rendererName;
}

void Mesh3dRenderer::RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet) {
	return;
	/*
	for (auto& renderQueue : renderQueues) {
		if (renderQueue.first == "Skybox") {
			continue;
		}

		for (auto& shaderUuid : renderQueue.second.shaders) {
			ShaderAsset& shader = *engineCore->assetManager->GetAsset<ShaderAsset>(shaderUuid);
			for (auto& materialUuid : shader.materials) {
				MaterialAsset& material = *engineCore->assetManager->GetAsset<MaterialAsset>(materialUuid);
				for (auto& renderable : material.renderables) {
					ECS::Entity entity = renderable.first;
					Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;

					auto graphicsCore = engineCore->GetGraphicsCore();
					commandBuffer->BindVertexArrayObject(submesh.vertexArrayObject);

					auto& registry = entity.GetScene()->GetEntityRegistry();
					entt::entity entityHandle = entity.GetHandle();
					auto& transformComponent = registry.get<TransformComponent>(entityHandle);
					auto& meshRendererComponent = registry.get<MeshRendererComponent>(entityHandle);
					glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
					meshRendererComponent.perDrawUniformBuffer->UpdateBuffer(&modelMatrix);

					commandBuffer->DrawIndices(
						submesh.baseIndex,
						submesh.indexCount,
						1,
						submesh.baseVertex
					);
				}
			}
		}
	}*/
}

void Mesh3dRenderer::RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* queueName) {
	RenderQueueContainer& renderQueue = renderQueues[renderQueueMap[queueName]];

	GraphicsAPI::GraphicsPipeline* graphicsPipeline = nullptr;
	Assets::AssetManager* assetManager = engineCore->assetManager;

	for (size_t sortIndex = 0; sortIndex < renderQueue.renderSortData.size(); ++sortIndex) {
		size_t renderTaskIndex = renderQueue.renderSortData[sortIndex].renderTaskIndex;
		Grindstone::RenderTask& renderTask = renderQueue.renderTasks[renderTaskIndex];
		if (graphicsPipeline != renderTask.pipeline) {
			graphicsPipeline = renderTask.pipeline;
			commandBuffer->BindGraphicsPipeline(renderTask.pipeline);
		}

		commandBuffer->BindVertexArrayObject(renderTask.vertexArrayObject);
		std::array<GraphicsAPI::DescriptorSet*, 3> descriptors = { renderTask.materialDescriptorSet, engineDescriptorSet, renderTask.perDrawDescriptorSet };
		commandBuffer->BindGraphicsDescriptorSet(
			graphicsPipeline,
			descriptors.data(),
			static_cast<uint32_t>(descriptors.size())
		);

		commandBuffer->DrawIndices(
			renderTask.baseIndex,
			renderTask.indexCount,
			1,
			renderTask.baseVertex
		);
	}
}

void Mesh3dRenderer::CacheRenderTasksAndFrustumCull(glm::vec3 eyePosition, entt::registry& registry) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	for(auto& renderQueue : renderQueues) {
		renderQueue.renderTasks.clear();
		renderQueue.renderTasks.reserve(8000);
	}

	auto view = registry.view<const TransformComponent, const MeshComponent, const MeshRendererComponent>();
	view.each([&](
		const TransformComponent& transformComponent,
		const MeshComponent& meshComponent,
		const MeshRendererComponent& meshRenderComponent
	) {
		// Add to Render Queue
		Mesh3dAsset* meshAsset = assetManager->GetAsset(meshComponent.mesh);

		if (meshAsset == nullptr) {
			return;
		}

		// Early Frustum Cull

		Math::Matrix4 transform = transformComponent.GetTransformMatrix();
		meshRenderComponent.perDrawUniformBuffer->UpdateBuffer(&transform);

		glm::vec3 offset = transformComponent.position - eyePosition;
		// Distance squared is used to sort. It's squared because we don't need exact distance to sort, and it's an expensive function.
		float distanceSquared = (offset.x * offset.x) + (offset.y * offset.y) + (offset.z * offset.z);
		uint32_t sortData = static_cast<uint32_t>(distanceSquared * 100.0f);

		auto& materials = meshRenderComponent.materials;
		for (Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
			if (submesh.materialIndex > materials.size()) {
				continue;
			}

			MaterialAsset* materialAsset = assetManager->GetAsset(materials[submesh.materialIndex]);
			if (materialAsset == nullptr) {
				continue;
			}

			ShaderAsset* shaderAsset = assetManager->GetAsset<ShaderAsset>(materialAsset->shaderUuid);
			if (shaderAsset == nullptr) {
				continue;
			}

			RenderTask renderTask{};
			renderTask.materialDescriptorSet = materialAsset->descriptorSet;
			renderTask.pipeline = shaderAsset->pipeline;
			renderTask.vertexArrayObject = meshAsset->vertexArrayObject;
			renderTask.indexCount = submesh.indexCount;
			renderTask.baseVertex = submesh.baseVertex;
			renderTask.baseIndex = submesh.baseIndex;
			renderTask.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet;
			renderTask.sortData = sortData;

			RenderQueueIndex renderQueueIndex = shaderAsset->renderQueue;

			if (renderQueueIndex == INVALID_RENDER_QUEUE || renderQueueIndex >= renderQueues.size()) {
				continue;
			}

			renderQueues[renderQueueIndex].renderTasks.emplace_back(renderTask);
		}
	});
}

static int CompareRenderSort(const RenderSortData& a, const RenderSortData& b) {
	return a.sortData > b.sortData;
}

static int CompareReverseRenderSort(const RenderSortData& a, const RenderSortData& b) {
	return a.sortData < b.sortData;
}

void Mesh3dRenderer::SortQueues() {
	for (RenderQueueContainer& renderQueue : renderQueues) {
		std::vector<RenderSortData>& renderSortData = renderQueue.renderSortData;
		RenderTaskList& renderTasks = renderQueue.renderTasks;

		renderSortData.resize(renderTasks.size());

		// Build sort data list
		for (size_t i = 0; i < renderSortData.size(); ++i) {
			renderSortData[i].renderTaskIndex = i;
			renderSortData[i].sortData = renderTasks[i].sortData;
		}

		// Sort the list
		if (renderQueue.sortMode == DrawSortMode::DistanceFrontToBack) {
			std::sort(renderSortData.begin(), renderSortData.end(), CompareRenderSort);
		}
		else {
			std::sort(renderSortData.begin(), renderSortData.end(), CompareReverseRenderSort);
		}
	}
}
