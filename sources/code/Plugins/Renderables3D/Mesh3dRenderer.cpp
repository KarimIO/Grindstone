#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};

Grindstone::Mesh3dRenderer::Mesh3dRenderer(EngineCore* engineCore) {
	this->engineCore = engineCore;
}

void Mesh3dRenderer::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	engineDescriptorSet = descriptorSet;
}

std::string& Mesh3dRenderer::GetName() {
	return rendererName;
}

void Mesh3dRenderer::RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet) {
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
					/*std::vector<GraphicsAPI::DescriptorSet*> descriptors = {material.descriptorSet, engineDescriptorSet, meshRendererComponent.perDrawDescriptorSet};
					commandBuffer->BindDescriptorSet(
						shader.pipeline,
						descriptors.data(),
						static_cast<uint32_t>(descriptors.size())
					);*/

					commandBuffer->DrawIndices(
						submesh.baseIndex,
						submesh.indexCount,
						1,
						submesh.baseVertex
					);
				}
			}
		}
	}
}

void Mesh3dRenderer::RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, RenderQueueContainer& renderQueue) {
	for (Uuid shaderUuid : renderQueue.shaders) {
		ShaderAsset& shader = *engineCore->assetManager->GetAsset<ShaderAsset>(shaderUuid);
		RenderShader(commandBuffer, shader);
	}
}

void Mesh3dRenderer::RenderQueueImmediate(RenderQueueContainer& renderQueue) {
	for (Uuid shaderUuid : renderQueue.shaders) {
		ShaderAsset& shader = *engineCore->assetManager->GetAsset<ShaderAsset>(shaderUuid);
		RenderShaderImmediate(shader);
	}
}

void Mesh3dRenderer::RenderShader(GraphicsAPI::CommandBuffer* commandBuffer, ShaderAsset& shader) {
	commandBuffer->BindPipeline(shader.pipeline);

	for (auto materialUuid : shader.materials) {
		MaterialAsset& material = *engineCore->assetManager->GetAsset<MaterialAsset>(materialUuid);
		RenderMaterial(commandBuffer, shader.pipeline, material);
	}
}

void Mesh3dRenderer::RenderMaterial(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::Pipeline* pipeline, MaterialAsset& material) {
	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;
		RenderSubmesh(commandBuffer, pipeline, material.descriptorSet, entity, submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::Pipeline* pipeline, GraphicsAPI::DescriptorSet* materialDescriptorSet, ECS::Entity rendererEntity, Mesh3dAsset::Submesh& submesh3d) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	commandBuffer->BindVertexArrayObject(submesh3d.vertexArrayObject);

	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.get<TransformComponent>(entity);
	auto& meshRendererComponent = registry.get<MeshRendererComponent>(entity);
	glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
	meshRendererComponent.perDrawUniformBuffer->UpdateBuffer(&modelMatrix);
	std::vector<GraphicsAPI::DescriptorSet*> descriptors = { materialDescriptorSet, engineDescriptorSet, meshRendererComponent.perDrawDescriptorSet };
	commandBuffer->BindDescriptorSet(
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

void Mesh3dRenderer::RenderShaderImmediate(ShaderAsset& shader) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	graphicsCore->BindPipeline(shader.pipeline);
	// mesh3dBufferObject->Bind();
	for (auto materialUuid : shader.materials) {
		MaterialAsset& material = *engineCore->assetManager->GetAsset<MaterialAsset>(materialUuid);
		RenderMaterialImmediate(material);
	}
}

void Mesh3dRenderer::RenderMaterialImmediate(MaterialAsset& material) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	/*
	if (material.uniformBufferObject) {
		material.uniformBufferObject->Bind();
	}
	*/
	/*
	if (material.textureBinding) {
		graphicsCore->BindTexture(material.textureBinding);
	}
	*/

	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;
		RenderSubmeshImmediate(entity, submesh);
	}
}

void Mesh3dRenderer::RenderSubmeshImmediate(ECS::Entity rendererEntity, Mesh3dAsset::Submesh& submesh3d) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	graphicsCore->BindVertexArrayObject(submesh3d.vertexArrayObject);
	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.get<TransformComponent>(entity);
	auto& meshRendererComponent = registry.get<MeshRendererComponent>(entity);
	glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
	meshRendererComponent.perDrawUniformBuffer->UpdateBuffer(&modelMatrix);

	graphicsCore->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		false,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
