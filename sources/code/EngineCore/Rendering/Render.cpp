#if 0
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;


std::array<glm::vec3, 8> cubeVertices = {
	glm::vec3(-1.0, -1.0,  1.0), glm::vec3( 1.0, -1.0,  1.0),
	glm::vec3( 1.0,  1.0,  1.0), glm::vec3(-1.0,  1.0,  1.0),
	glm::vec3(-1.0, -1.0, -1.0), glm::vec3( 1.0, -1.0, -1.0),
	glm::vec3( 1.0,  1.0, -1.0), glm::vec3(-1.0,  1.0, -1.0)
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

void Render(
	GraphicsAPI::Core *core,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix
) {
	VertexBufferLayout layout({
		{
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	});

	VertexBuffer::CreateInfo vboCi;
	vboCi.content = cubeVertices.data();
	vboCi.count = cubeVertices.size();
	vboCi.size = vboCi.count * sizeof(glm::vec3);
	vboCi.layout = &layout;
	VertexBuffer* vbo = core->CreateVertexBuffer(vboCi);

	IndexBuffer::CreateInfo iboCi;
	iboCi.content = cubeIndices.data();
	iboCi.count = cubeIndices.size();
	iboCi.size = sizeof(uint32_t) * iboCi.count;
	IndexBuffer* ibo = core->CreateIndexBuffer(iboCi);

	VertexArrayObject::CreateInfo vaoCi;
	vaoCi.vertex_buffer_count = 1;
	vaoCi.vertex_buffers = &vbo;
	vaoCi.index_buffer = ibo;
	VertexArrayObject* vao = core->CreateVertexArrayObject(vaoCi);

	/*UniformBufferBinding::CreateInfo ubbCi;
	ubbCi.binding = 0;
	ubbCi.shaderLocation = "UniformBuffer";
	ubbCi.size = sizeof(float) * 16;
	ubbCi.stages = ShaderStageBit::AllGraphics;
	UniformBufferBinding* ubb = core->CreateUniformBufferBinding(ubbCi);

	UniformBuffer::CreateInfo ubCi;
	ubCi.binding = ubb;
	ubCi.isDynamic = true;
	ubCi.size = sizeof(float) * 16;
	UniformBuffer* ubo = core->CreateUniformBuffer(ubCi);
	ubo->updateBuffer();*/

	core->Clear(ClearMode::All);
	core->DrawImmediateIndexed(GeometryType::Triangles, true, 0, 0, 32);
	
	// RenderLights();
	// PostProcess();
}
#endif