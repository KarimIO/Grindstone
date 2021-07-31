#include <array>
#include <glm/glm.hpp>
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
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

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
// Material* myMaterial;

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
		VertexBufferLayout vertexPositionLayout({
			{
				Grindstone::GraphicsAPI::VertexFormat::Float3,
				"vertexPosition",
				false,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		});

		/*VertexBuffer::CreateInfo vboCi{};
		vboCi.content = cubeVertexPositions.data();
		vboCi.count = cubeVertexPositions.size();
		vboCi.size = vboCi.count * sizeof(glm::vec3);
		vboCi.layout = &vertexPositionLayout;
		vbo = core->CreateVertexBuffer(vboCi);*/

		VertexBufferLayout vertexTexCoordLayout({
			{
				Grindstone::GraphicsAPI::VertexFormat::Float2,
				"vertexTexCoord",
				false,
				Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
			}
		});

		/*VertexBuffer::CreateInfo vboTexCi{};
		vboTexCi.content = cubeVertexTexCoords.data();
		vboTexCi.count = cubeVertexTexCoords.size();
		vboTexCi.size = vboTexCi.count * sizeof(glm::vec3);
		vboTexCi.layout = &vertexTexCoordLayout;
		vboTex = core->CreateVertexBuffer(vboTexCi);

		IndexBuffer::CreateInfo iboCi{};
		iboCi.content = cubeIndices.data();
		iboCi.count = cubeIndices.size();
		iboCi.size = sizeof(uint32_t) * iboCi.count;
		IndexBuffer* ibo = core->CreateIndexBuffer(iboCi);

		std::vector<VertexBuffer*> vbs = { vbo, vboTex };

		VertexArrayObject::CreateInfo vaoCi{};
		vaoCi.vertex_buffer_count = vbs.size();
		vaoCi.vertex_buffers = vbs.data();
		vaoCi.index_buffer = ibo;
		vao = core->CreateVertexArrayObject(vaoCi);*/

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

		// auto materialManager = EngineCore::GetInstance().materialManager;
		// myMaterial = &materialManager->LoadMaterial("../assets/New Material.gmat");

		Mesh3dManager* mesh3dManager = EngineCore::GetInstance().mesh3dManager;
		mesh3dManager->LoadMesh3d("../assets/models/sphere.gmf");

		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::mat4(1);

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	core->Clear(ClearMode::All, clearColor, 1);

	/*myMaterial->shader->pipeline->bind();
	if (myMaterial->uniformBufferObject) {
		myMaterial->uniformBufferObject->updateBuffer(myMaterial->buffer);
		myMaterial->uniformBufferObject->bind();
	}
	core->BindTexture(myMaterial->textureBinding);*/
	// pipeline->bind();
	// ubo->updateBuffer(&engineUboStruct);
	// ubo->bind();
	// vao->bind();
	// core->DrawImmediateIndexed(GeometryType::Triangles, true, 0, 0, 36);
	
	// RenderLights();
	// PostProcess();
}
