#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
using namespace Grindstone;

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
	material.shader->pipeline->bind();
	if (material.uniformBufferObject) {
		material.uniformBufferObject->updateBuffer(material.buffer);
		material.uniformBufferObject->bind();
	}

	GraphicsAPI::Core* core = EngineCore::GetInstance().getGraphicsCore();
	core->BindTexture(material.textureBinding);

	for (void* submesh : material.renderables) {
		RenderSubmesh(*(Mesh3d::Submesh*)submesh);
	}
}

void Mesh3dRenderer::RenderSubmesh(Mesh3d::Submesh& submesh3d) {
	Mesh3d mesh3d = submesh3d.mesh;

	GraphicsAPI::Core* core = EngineCore::GetInstance().getGraphicsCore();
	core->BindVertexArrayObject(mesh3d.vertexArrayObject);
	core->DrawImmediateIndexed(
		GraphicsAPI::GeometryType::Triangles,
		true,
		submesh3d.baseVertex,
		submesh3d.baseIndex,
		submesh3d.indexCount
	);
}
