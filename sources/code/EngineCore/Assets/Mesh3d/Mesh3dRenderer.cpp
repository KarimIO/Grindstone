#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
using namespace Grindstone;

Grindstone::Mesh3dRenderer::Mesh3dRenderer() {
	errorMaterial = &EngineCore::GetInstance().materialManager->LoadMaterial(this, "Error.gmat");
}

void Mesh3dRenderer::RenderQueue(RenderQueueContainer& renderQueue) {
	for (Shader* shader : renderQueue.shaders) {
		RenderShader(*shader);
	}
}

void Mesh3dRenderer::RenderShader(Shader& shader) {
	shader.pipeline->bind();
	for (auto& material : shader.materials) {
		RenderMaterial(*material);
	}
}

void Mesh3dRenderer::RenderMaterial(Material& material) {
	if (material.uniformBufferObject) {
		material.uniformBufferObject->UpdateBuffer(material.buffer);
		material.uniformBufferObject->Bind();
	}

	GraphicsAPI::Core* core = EngineCore::GetInstance().GetGraphicsCore();
	core->BindTexture(material.textureBinding);

	for (void* renderable : material.renderables) {
		Mesh3d::Submesh& submesh = *(Mesh3d::Submesh *)renderable;
		RenderSubmesh(submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(Mesh3d::Submesh& submesh3d) {
	Mesh3d& mesh3d = *submesh3d.mesh;

	GraphicsAPI::Core* core = EngineCore::GetInstance().GetGraphicsCore();
	core->BindVertexArrayObject(mesh3d.vertexArrayObject);
	core->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		false,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
