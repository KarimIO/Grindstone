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

Grindstone::Mesh3dRenderer::Mesh3dRenderer(GraphicsAPI::Core* graphicsCore) {
	this->graphicsCore = graphicsCore;

	UniformBufferBinding::CreateInfo mesh3dBufferBindingCi{};
	mesh3dBufferBindingCi.binding = 1;
	mesh3dBufferBindingCi.shaderLocation = "MeshUbo";
	mesh3dBufferBindingCi.size = sizeof(Mesh3dUbo);
	mesh3dBufferBindingCi.stages = ShaderStageBit::AllGraphics;
	mesh3dBufferBinding = graphicsCore->CreateUniformBufferBinding(mesh3dBufferBindingCi);

	UniformBuffer::CreateInfo mesh3dBufferObjectCi{};
	mesh3dBufferObjectCi.binding = mesh3dBufferBinding;
	mesh3dBufferObjectCi.isDynamic = true;
	mesh3dBufferObjectCi.size = sizeof(Mesh3dUbo);
	mesh3dBufferObject = graphicsCore->CreateUniformBuffer(mesh3dBufferObjectCi);
}

void Mesh3dRenderer::RenderQueue(RenderQueueContainer& renderQueue) {
	for (ShaderAsset* shader : renderQueue.shaders) {
		RenderShader(*shader);
	}
}

void Mesh3dRenderer::RenderShader(ShaderAsset& shader) {
	graphicsCore->BindPipeline(shader.pipeline);
	mesh3dBufferObject->Bind();
	for (auto& material : shader.materials) {
		RenderMaterial(material);
	}
}

void Mesh3dRenderer::RenderMaterial(MaterialAsset& material) {
	if (material.uniformBufferObject) {
		material.uniformBufferObject->UpdateBuffer(material.buffer);
		material.uniformBufferObject->Bind();
	}

	if (material.textureBinding) {
		graphicsCore->BindTexture(material.textureBinding);
	}

	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3dAsset::Submesh& submesh = *(Mesh3dAsset::Submesh*)renderable.second;
		RenderSubmesh(entity, submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(ECS::Entity rendererEntity, Mesh3dAsset::Submesh& submesh3d) {
	graphicsCore->BindVertexArrayObject(submesh3d.vertexArrayObject);
	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.get<TransformComponent>(entity);
	glm::mat4 modelMatrix = 
		glm::translate(transformComponent.position) *
		glm::toMat4(transformComponent.rotation) *
		glm::scale(transformComponent.scale);
	mesh3dBufferObject->UpdateBuffer(&modelMatrix);

	graphicsCore->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		false,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
