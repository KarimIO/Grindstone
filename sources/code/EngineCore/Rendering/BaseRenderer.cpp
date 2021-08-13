#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/Pipeline.hpp"
#include "BaseRenderer.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"
#include "EngineCore/Assets/Shaders/Shader.hpp"
#include "EngineCore/Assets/Materials/Material.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

// #define TESTING_MESH

std::array<glm::vec3, 8> cubeVertexPositions = {
	glm::vec3(-1, -1,  1), glm::vec3(1, -1,  1),
	glm::vec3(1,  1,  1), glm::vec3(-1,  1,  1),
	glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1),
	glm::vec3(1,  1, -1), glm::vec3(-1,  1, -1)
};

std::array<glm::vec2, 8> cubeVertexTexCoords = {
	glm::vec2(0, 0), glm::vec2(1, 0),
	glm::vec2(1, 0), glm::vec2(1, 1),
	glm::vec2(1, 1), glm::vec2(0, 0),
	glm::vec2(0, 0), glm::vec2(1, 0),
};

std::array<uint32_t, 36> cubeIndices = {
	// front
	2, 1, 0,
	0, 3, 2,
	// right
	6, 5, 1,
	1, 2, 6,
	// back
	5, 6, 7,
	7, 4, 5,
	// left
	3, 0, 4,
	4, 7, 3,
	// bottom
	1, 5, 4,
	4, 0, 1,
	// top
	6, 2, 3,
	3, 7, 6
};

bool isFirst = true;
Pipeline* pipeline;
VertexBuffer* vbo;
VertexBuffer* vboTex;
VertexArrayObject* vao;
UniformBufferBinding* ubb;
UniformBuffer* ubo;
Material* myMaterial;
Mesh3d* mesh3d;

struct EngineUboStruct {
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;
};

void Grindstone::BaseRender(
	GraphicsAPI::Core *core,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix
) {
	if (isFirst) {
		UniformBufferBinding::CreateInfo ubbCi{};
		ubbCi.binding = 0;
		ubbCi.shaderLocation = "EngineUbo";
		ubbCi.size = sizeof(glm::mat4) * 3;
		ubbCi.stages = ShaderStageBit::AllGraphics;
		ubb = core->CreateUniformBufferBinding(ubbCi);

		UniformBuffer::CreateInfo ubCi{};
		ubCi.binding = ubb;
		ubCi.isDynamic = true;
		ubCi.size = sizeof(glm::mat4) * 3;
		ubo = core->CreateUniformBuffer(ubCi);

		auto materialManager = EngineCore::GetInstance().materialManager;
		myMaterial = &materialManager->LoadMaterial("../assets/New Material.gmat");
#ifndef TESTING_MESH
		Mesh3dManager* mesh3dManager = EngineCore::GetInstance().mesh3dManager;
		mesh3d = &mesh3dManager->LoadMesh3d("../assets/models/sphere.gmf");
#endif
		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::scale(glm::vec3(0.05f)) * glm::mat4(1);

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	core->Clear(ClearMode::All, clearColor, 1);
	// EngineCore::GetInstance().assetRendererManager->RenderQueue("Opaque");

#ifndef TESTING_MESH
	myMaterial->shader->pipeline->bind();
	if (myMaterial->uniformBufferObject) {
		myMaterial->uniformBufferObject->updateBuffer(myMaterial->buffer);
		myMaterial->uniformBufferObject->bind();
	}
	// core->BindTexture(myMaterial->textureBinding);

	// pipeline->bind();
	ubo->updateBuffer(&engineUboStruct);
	ubo->bind();
	mesh3d->vertexArrayObject->Bind();
	Mesh3d::Submesh& submesh = mesh3d->submeshes[0];
	core->DrawImmediateIndexed(GeometryType::Triangles, false, submesh.baseVertex, submesh.baseIndex, submesh.indexCount);
#endif
	
	// RenderLights();
	// PostProcess();
}
