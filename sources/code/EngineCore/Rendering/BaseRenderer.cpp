#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/Pipeline.hpp"
#include "BaseRenderer.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

std::array<glm::vec3, 8> cubeVertices = {
	glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1),
	glm::vec3( 1,  1,  1), glm::vec3(-1,  1,  1),
	glm::vec3(-1, -1, -1), glm::vec3( 1, -1, -1),
	glm::vec3( 1,  1, -1), glm::vec3(-1,  1, -1)
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

/*
std::string vertexShader = "#version 330 core\n\
layout(location = 0) in vec3 vertexPosition;\n\
layout (std140) uniform Matrices {\n\
	mat4 proj;\n\
};\n\
void main() {\n\
	gl_Position = proj * vec4(vertexPosition, 1.0);\n\
}";

std::string fragmentShader = "#version 330 core\n\
out vec4 color;\n\
void main() {\n\
	color = vec4(1, 0, 0, 1);\n\
}";
*/

bool isFirst = true;
Pipeline* pipeline;
VertexBuffer* vbo;
VertexArrayObject* vao;
UniformBufferBinding* ubb;
UniformBuffer* ubo;
Material* myMaterial;

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
		VertexBufferLayout layout({
			{
				Grindstone::GraphicsAPI::VertexFormat::Float3,
				"vertexPosition",
				false,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		});

		VertexBuffer::CreateInfo vboCi{};
		vboCi.content = cubeVertices.data();
		vboCi.count = cubeVertices.size();
		vboCi.size = vboCi.count * sizeof(glm::vec3);
		vboCi.layout = &layout;
		vbo = core->CreateVertexBuffer(vboCi);

		IndexBuffer::CreateInfo iboCi{};
		iboCi.content = cubeIndices.data();
		iboCi.count = cubeIndices.size();
		iboCi.size = sizeof(uint32_t) * iboCi.count;
		IndexBuffer* ibo = core->CreateIndexBuffer(iboCi);

		VertexArrayObject::CreateInfo vaoCi{};
		vaoCi.vertex_buffer_count = 1;
		vaoCi.vertex_buffers = &vbo;
		vaoCi.index_buffer = ibo;
		vao = core->CreateVertexArrayObject(vaoCi);

		
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

		/*
		
		std::vector<ShaderStageCreateInfo> shaderStages;
		shaderStages.resize(2);
		shaderStages[0] = ShaderStageCreateInfo{ "a.vert", vertexShader.c_str(), (unsigned int)vertexShader.size(), ShaderStage::Vertex };
		shaderStages[1] = ShaderStageCreateInfo{ "a.frag", fragmentShader.c_str(), (unsigned int)fragmentShader.size(), ShaderStage::Fragment };

		
		Pipeline::CreateInfo pipelineCi{};
		pipelineCi.primitiveType = GeometryType::Triangles;
		pipelineCi.cullMode = CullMode::None;
		pipelineCi.renderPass;
		pipelineCi.width = 800;
		pipelineCi.height = 600;
		pipelineCi.scissorX = 0;
		pipelineCi.scissorY = 0;
		pipelineCi.scissorW = 800;
		pipelineCi.scissorH = 600;
		pipelineCi.shaderStageCreateInfos = shaderStages.data();
		pipelineCi.shaderStageCreateInfoCount = shaderStages.size();

		pipelineCi.uniformBufferBindings = &ubb;
		pipelineCi.uniformBufferBindingCount = 1;

		pipelineCi.textureBindings = 0;
		pipelineCi.textureBindingCount = 0;

		pipelineCi.vertexBindings = &layout;
		pipelineCi.vertexBindingsCount = 1;
		pipeline = core->CreatePipeline(pipelineCi);
		*/

		auto materialManager = EngineCore::GetInstance().materialManager;
		myMaterial = &materialManager->LoadMaterial("../assets/New Material.gmat");

		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::mat4(1);

	static float clearColor[4] = { 0.2f, 0.6f, 0, 1.f };
	core->Clear(ClearMode::All, clearColor, 1);
	myMaterial->shader->pipeline->bind();
	// pipeline->bind();
	ubo->updateBuffer(&engineUboStruct);
	vao->bind();
	core->DrawImmediateIndexed(GeometryType::Triangles, true, 0, 0, 32);
	
	// RenderLights();
	// PostProcess();
}
