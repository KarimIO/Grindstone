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
#include "EngineCore/Assets/Mesh3d/Mesh3dRenderer.hpp"
#include "EngineCore/Assets/Shaders/Shader.hpp"
#include "EngineCore/Assets/Materials/Material.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

bool isFirst = true;
UniformBufferBinding* globalUniformBufferBinding;
UniformBuffer* globalUniformBufferObject;

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
		UniformBufferBinding::CreateInfo globalUniformBufferBindingCi{};
		globalUniformBufferBindingCi.binding = 0;
		globalUniformBufferBindingCi.shaderLocation = "EngineUbo";
		globalUniformBufferBindingCi.size = sizeof(glm::mat4) * 3;
		globalUniformBufferBindingCi.stages = ShaderStageBit::AllGraphics;
		globalUniformBufferBinding = core->CreateUniformBufferBinding(globalUniformBufferBindingCi);

		UniformBuffer::CreateInfo ubCi{};
		ubCi.binding = globalUniformBufferBinding;
		ubCi.isDynamic = true;
		ubCi.size = sizeof(glm::mat4) * 3;
		globalUniformBufferObject = core->CreateUniformBuffer(ubCi);

		auto& engineCore = EngineCore::GetInstance();
		auto materialManager = engineCore.materialManager;
		auto mesh3dRenderer = engineCore.mesh3dRenderer;
		Material& myMaterial = materialManager->LoadMaterial(mesh3dRenderer, "../assets/New Material.gmat");

		Mesh3dManager* mesh3dManager = EngineCore::GetInstance().mesh3dManager;
		Mesh3d& mesh3d = mesh3dManager->LoadMesh3d("../assets/models/sphere.gmf");
		myMaterial.renderables.push_back(&mesh3d.submeshes[0]);

		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::scale(glm::vec3(0.05f)) * glm::mat4(1);

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	core->Clear(ClearMode::All, clearColor, 1);

	globalUniformBufferObject->UpdateBuffer(&engineUboStruct);
	globalUniformBufferObject->Bind();
	EngineCore::GetInstance().assetRendererManager->RenderQueue("Opaque");
	
	// RenderLights();
	// PostProcess();
}
