#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};

Grindstone::Mesh3dRenderer::Mesh3dRenderer(EngineCore* engineCore) {
	this->engineCore = engineCore;
	auto graphicsCore = engineCore->GetGraphicsCore();

	UniformBuffer::CreateInfo mesh3dBufferObjectCi{};
	mesh3dBufferObjectCi.debugName = "Mesh Uniform Buffer";
	mesh3dBufferObjectCi.isDynamic = true;
	mesh3dBufferObjectCi.size = sizeof(Mesh3dUbo);
	mesh3dBufferObject = graphicsCore->CreateUniformBuffer(mesh3dBufferObjectCi);

	DescriptorSet::Binding meshUniformBufferBinding{};
	meshUniformBufferBinding.bindingIndex = 1;
	meshUniformBufferBinding.bindingType = BindingType::UniformBuffer;
	meshUniformBufferBinding.count = 1;
	meshUniformBufferBinding.itemPtr = mesh3dBufferObject;

	DescriptorSetLayout::Binding meshDescriptorLayoutBinding{};
	meshDescriptorLayoutBinding.bindingId = 1;
	meshDescriptorLayoutBinding.count = 1;
	meshDescriptorLayoutBinding.type = BindingType::UniformBuffer;
	meshDescriptorLayoutBinding.stages = ShaderStageBit::AllGraphics;

	DescriptorSetLayout::CreateInfo meshDescriptorSetLayoutCi{};
	meshDescriptorSetLayoutCi.bindingCount = 1;
	meshDescriptorSetLayoutCi.bindings = &meshDescriptorLayoutBinding;
	DescriptorSetLayout* meshDescriptorLayout = graphicsCore->CreateDescriptorSetLayout(meshDescriptorSetLayoutCi);

	DescriptorSet::CreateInfo meshUniformBufferCi{};
	meshUniformBufferCi.bindingCount = 1;
	meshUniformBufferCi.bindings = &meshUniformBufferBinding;
	meshUniformBufferCi.layout = meshDescriptorLayout;
	DescriptorSet* meshDescriptor = graphicsCore->CreateDescriptorSet(meshUniformBufferCi);
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
		RenderMaterial(commandBuffer, material);
	}
}

void Mesh3dRenderer::RenderMaterial(GraphicsAPI::CommandBuffer* commandBuffer, MaterialAsset& material) {
	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;
		RenderSubmesh(commandBuffer, entity, submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(GraphicsAPI::CommandBuffer* commandBuffer, ECS::Entity rendererEntity, Mesh3dAsset::Submesh& submesh3d) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	commandBuffer->BindVertexArrayObject(submesh3d.vertexArrayObject);

	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.get<TransformComponent>(entity);
	glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
	mesh3dBufferObject->UpdateBuffer(&modelMatrix);

	commandBuffer->DrawIndices(
		0,
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
	glm::mat4 modelMatrix = transformComponent.GetTransformMatrix();
	mesh3dBufferObject->UpdateBuffer(&modelMatrix);

	graphicsCore->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		false,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
