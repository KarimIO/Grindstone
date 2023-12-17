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
	auto assetManager = engineCore->assetManager;

	for (size_t i = 0; i < renderQueue.renderTasks.size(); ++i) {
		Grindstone::RenderTask& renderTask = renderQueue.renderTasks[i];
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

#if 0
void Mesh3dRenderer::RenderShader(GraphicsAPI::CommandBuffer* commandBuffer, ShaderAsset& shader) {
	commandBuffer->BindGraphicsPipeline(shader.pipeline);

	for (auto materialUuid : shader.materials) {
		MaterialAsset& material = *engineCore->assetManager->GetAsset<MaterialAsset>(materialUuid);
		RenderMaterial(commandBuffer, shader.pipeline, material);
	}
}

void Mesh3dRenderer::RenderMaterial(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::GraphicsPipeline* pipeline, MaterialAsset& material) {
	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;
		RenderSubmesh(commandBuffer, pipeline, material.descriptorSet, entity, submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::GraphicsPipeline* pipeline, GraphicsAPI::DescriptorSet* materialDescriptorSet, ECS::Entity rendererEntity, Mesh3dAsset::Submesh& submesh3d) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	commandBuffer->BindVertexArrayObject(submesh3d.vertexArrayObject);

	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.get<TransformComponent>(entity);
	auto& meshRendererComponent = registry.get<MeshRendererComponent>(entity);
	glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
	meshRendererComponent.perDrawUniformBuffer->UpdateBuffer(&modelMatrix);
	std::vector<GraphicsAPI::DescriptorSet*> descriptors = { materialDescriptorSet, engineDescriptorSet, meshRendererComponent.perDrawDescriptorSet };
	commandBuffer->BindGraphicsDescriptorSet(
		pipeline,
		descriptors.data(),
		static_cast<uint32_t>(descriptors.size())
	);

	commandBuffer->DrawIndices(
		submesh3d.baseIndex,
		submesh3d.indexCount,
		1,
		submesh3d.baseVertex
	);
}
#endif

void Mesh3dRenderer::CacheRenderTasksAndFrustumCull(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry) {
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
		Math::Matrix4 transform = transformComponent.GetTransformMatrix();
		meshRenderComponent.perDrawUniformBuffer->UpdateBuffer(&transform);

		// Early Frustum Cull

		// Add to Render Queue
		Mesh3dAsset* meshAsset = assetManager->GetAsset(meshComponent.mesh);

		if (meshAsset == nullptr) {
			return;
		}

		for(auto& submesh : meshAsset->submeshes) {
			MaterialAsset* materialAsset = assetManager->GetAsset(meshRenderComponent.materials[submesh.materialIndex]);
			if (materialAsset == nullptr) {
				continue;
			}

			ShaderAsset* shaderAsset = assetManager->GetAsset<ShaderAsset>(materialAsset->shaderUuid);
			if (shaderAsset == nullptr) {
				continue;
			}

			RenderQueueIndex renderQueueIndex = shaderAsset->renderQueue;
			RenderTask& renderTask = renderQueues[renderQueueIndex].renderTasks.emplace_back();
			renderTask.materialDescriptorSet = materialAsset->descriptorSet;
			renderTask.pipeline = shaderAsset->pipeline;
			renderTask.vertexArrayObject = meshAsset->vertexArrayObject;
			renderTask.indexCount = submesh.indexCount;
			renderTask.baseVertex = submesh.baseVertex;
			renderTask.baseIndex = submesh.baseIndex;
			renderTask.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet;
		}
	});

	// Sort here
}
