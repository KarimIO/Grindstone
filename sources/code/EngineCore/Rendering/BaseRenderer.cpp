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
UniformBufferBinding* globalUniformBufferBinding = nullptr;
UniformBuffer* globalUniformBufferObject = nullptr;
Framebuffer* gbuffer = nullptr;
RenderTarget* renderTargets = nullptr;
DepthTarget* depthTarget = nullptr;

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
		globalUniformBufferBindingCi.size = sizeof(EngineUboStruct);
		globalUniformBufferBindingCi.stages = ShaderStageBit::AllGraphics;
		globalUniformBufferBinding = core->CreateUniformBufferBinding(globalUniformBufferBindingCi);

		UniformBuffer::CreateInfo globalUniformBufferObjectCi{};
		globalUniformBufferObjectCi.binding = globalUniformBufferBinding;
		globalUniformBufferObjectCi.isDynamic = true;
		globalUniformBufferObjectCi.size = sizeof(EngineUboStruct);
		globalUniformBufferObject = core->CreateUniformBuffer(globalUniformBufferObjectCi);

		const uint32_t width = 800;
		const uint32_t height = 600;
		std::vector<Grindstone::GraphicsAPI::RenderTarget::CreateInfo> gbufferImagesCreateInfo;
		gbufferImagesCreateInfo.reserve(3);
		gbufferImagesCreateInfo.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, width, height); // R  G  B matID
		gbufferImagesCreateInfo.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R16G16B16A16, width, height); // nX nY nZ
		gbufferImagesCreateInfo.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, width, height); // sR sG sB Roughness
		renderTargets = core->CreateRenderTarget(gbufferImagesCreateInfo.data(), (uint32_t)gbufferImagesCreateInfo.size());

		Grindstone::GraphicsAPI::DepthTarget::CreateInfo depthImageCreateInfo(Grindstone::GraphicsAPI::DepthFormat::D24_STENCIL_8, width, height, false, false);
		depthTarget = core->CreateDepthTarget(depthImageCreateInfo);

		Grindstone::GraphicsAPI::Framebuffer::CreateInfo gbufferCreateInfo{};
		gbufferCreateInfo.renderTargetLists = &renderTargets;
		gbufferCreateInfo.numRenderTargetLists = 1;
		gbufferCreateInfo.depthTarget = depthTarget;
		gbufferCreateInfo.renderPass = nullptr;
		gbuffer = core->CreateFramebuffer(gbufferCreateInfo);

		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::scale(glm::vec3(0.05f)) * glm::mat4(1);

	gbuffer->Bind(true);

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	core->Clear(ClearMode::All, clearColor, 1);

	globalUniformBufferObject->UpdateBuffer(&engineUboStruct);
	globalUniformBufferObject->Bind();
	EngineCore::GetInstance().assetRendererManager->RenderQueue("Opaque");

	gbuffer->Unbind();
	
	// RenderLights();
	// PostProcess();
}
