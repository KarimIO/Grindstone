#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};

Grindstone::Mesh3dRenderer::Mesh3dRenderer() {
	auto core = EngineCore::GetInstance().GetGraphicsCore();

	UniformBufferBinding::CreateInfo mesh3dBufferBindingCi{};
	mesh3dBufferBindingCi.binding = 1;
	mesh3dBufferBindingCi.shaderLocation = "MeshUbo";
	mesh3dBufferBindingCi.size = sizeof(Mesh3dUbo);
	mesh3dBufferBindingCi.stages = ShaderStageBit::AllGraphics;
	mesh3dBufferBinding = core->CreateUniformBufferBinding(mesh3dBufferBindingCi);

	UniformBuffer::CreateInfo mesh3dBufferObjectCi{};
	mesh3dBufferObjectCi.binding = mesh3dBufferBinding;
	mesh3dBufferObjectCi.isDynamic = true;
	mesh3dBufferObjectCi.size = sizeof(Mesh3dUbo);
	mesh3dBufferObject = core->CreateUniformBuffer(mesh3dBufferObjectCi);
}

void Mesh3dRenderer::AddErrorMaterial() {
	auto materialImporter = EngineCore::GetInstance().materialImporter;
	errorMaterial = &materialImporter->LoadMaterial(this, "792d934c-78d5-4445-b0e8-fc2828eed098");
}

void Mesh3dRenderer::RenderQueue(RenderQueueContainer& renderQueue) {
	for (Shader* shader : renderQueue.shaders) {
		RenderShader(*shader);
	}
}

void Mesh3dRenderer::RenderShader(Shader& shader) {
	GraphicsAPI::Core* core = EngineCore::GetInstance().GetGraphicsCore();
	core->BindPipeline(shader.pipeline);
	mesh3dBufferObject->Bind();
	for (auto& material : shader.materials) {
		RenderMaterial(*material);
	}
}

void Mesh3dRenderer::RenderMaterial(Material& material) {
	if (material.uniformBufferObject) {
		material.uniformBufferObject->UpdateBuffer(material.buffer);
		material.uniformBufferObject->Bind();
	}

	if (material.textureBinding) {
		GraphicsAPI::Core* core = EngineCore::GetInstance().GetGraphicsCore();
		core->BindTexture(material.textureBinding);
	}

	for (auto& renderable : material.renderables) {
		ECS::Entity entity = renderable.first;
		Mesh3d::Submesh& submesh = *(Mesh3d::Submesh*)renderable.second;
		RenderSubmesh(entity, submesh);
	}
}

// TODO: Bind vao once for multiple MeshRenderers
void Mesh3dRenderer::RenderSubmesh(ECS::Entity rendererEntity, Mesh3d::Submesh& submesh3d) {
	Mesh3d& mesh3d = *submesh3d.mesh;

	GraphicsAPI::Core* core = EngineCore::GetInstance().GetGraphicsCore();

	core->BindVertexArrayObject(mesh3d.vertexArrayObject);
	auto& registry = rendererEntity.GetScene()->GetEntityRegistry();
	entt::entity entity = rendererEntity.GetHandle();
	auto& transformComponent = registry.Get<TransformComponent>(entity);
	glm::mat4 modelMatrix = 
		glm::translate(transformComponent.position) *
		glm::toMat4(transformComponent.rotation) *
		glm::scale(transformComponent.scale);
	mesh3dBufferObject->UpdateBuffer(&modelMatrix);

	core->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		false,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
