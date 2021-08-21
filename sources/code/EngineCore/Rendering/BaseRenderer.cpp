#include <array>
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
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Assets/Materials/Material.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/CoreComponents/Lights/PointLightComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

float lightPositions[] = {
	-1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	 1.0f, -1.0f
};

uint16_t lightIndices[] = {
	0, 1, 2,
	3, 2, 0
};

bool isFirst = true;
UniformBufferBinding* globalUniformBufferBinding = nullptr;
UniformBuffer* globalUniformBufferObject = nullptr;
UniformBufferBinding* lightUniformBufferBinding = nullptr;
UniformBuffer* lightUniformBufferObject = nullptr;
Framebuffer* gbuffer = nullptr;
Framebuffer* litHdrFramebuffer = nullptr;
RenderTarget* renderTargets = nullptr;
RenderTarget* litHdrImages = nullptr;
DepthTarget* depthTarget = nullptr;

struct EngineUboStruct {
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;
	glm::vec3 eyePos;
};

struct LightmapStruct {
	glm::vec3 lightColor = glm::vec3(3, 0.8, 0.4);
	float lightAttenuationRadius = 40.0f;
	glm::vec3 lightPosition = glm::vec3(1, 2, 1);
	float lightIntensity = 40.0f;
};
VertexArrayObject* planePostProcessVao = nullptr;
Pipeline* lightPipeline = nullptr;
Pipeline* tonemapPipeline = nullptr;

void RenderLights(entt::registry& registry) {
	auto core = EngineCore::GetInstance().GetGraphicsCore();

	core->BindPipeline(lightPipeline);
	litHdrFramebuffer->BindWrite(false);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.f };
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	core->SetImmediateBlending(BlendMode::Additive);
	gbuffer->BindTextures(2);

	auto view = registry.view<const TransformComponent, const PointLightComponent>();
	view.each([&](const TransformComponent& transformComponent, const PointLightComponent& pointLightComponent) {
		LightmapStruct lightmapStruct {
			pointLightComponent.color,
			pointLightComponent.attenuationRadius,
			transformComponent.position,
			pointLightComponent.intensity,
		};

		lightUniformBufferObject->UpdateBuffer(&lightmapStruct);
		lightUniformBufferObject->Bind();
		planePostProcessVao->Bind();
		core->DrawImmediateIndexed(GeometryType::Triangles, false, 0, 0, 6);
	});
}

void PostProcess() {
	auto core = EngineCore::GetInstance().GetGraphicsCore();

	core->BindPipeline(tonemapPipeline);
	core->BindDefaultFramebufferWrite(true);
	litHdrFramebuffer->BindRead();

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.f };
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	core->SetImmediateBlending(BlendMode::None);
	litHdrFramebuffer->BindTextures(1);
	planePostProcessVao->Bind();
	core->DrawImmediateIndexed(GeometryType::Triangles, false, 0, 0, 6);
}

void Grindstone::BaseRender(
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos
) {
	auto core = EngineCore::GetInstance().GetGraphicsCore();
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
		std::vector<RenderTarget::CreateInfo> gbufferImagesCreateInfo;
		gbufferImagesCreateInfo.reserve(4);
		gbufferImagesCreateInfo.emplace_back(ColorFormat::R16G16B16A16, width, height); // X Y Z
		gbufferImagesCreateInfo.emplace_back(ColorFormat::R8G8B8A8, width, height); // R  G  B matID
		gbufferImagesCreateInfo.emplace_back(ColorFormat::R16G16B16A16, width, height); // nX nY nZ
		gbufferImagesCreateInfo.emplace_back(ColorFormat::R8G8B8A8, width, height); // sR sG sB Roughness
		renderTargets = core->CreateRenderTarget(gbufferImagesCreateInfo.data(), (uint32_t)gbufferImagesCreateInfo.size());

		DepthTarget::CreateInfo depthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false);
		depthTarget = core->CreateDepthTarget(depthImageCreateInfo);

		Framebuffer::CreateInfo gbufferCreateInfo{};
		gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
		gbufferCreateInfo.renderTargetLists = &renderTargets;
		gbufferCreateInfo.numRenderTargetLists = 1;
		gbufferCreateInfo.depthTarget = depthTarget;
		gbufferCreateInfo.renderPass = nullptr;
		gbuffer = core->CreateFramebuffer(gbufferCreateInfo);

		RenderTarget::CreateInfo litHdrImagesCreateInfo
			= { Grindstone::GraphicsAPI::ColorFormat::R32G32B32, width, height };
		litHdrImages = core->CreateRenderTarget(&litHdrImagesCreateInfo, 1);

		Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
		litHdrFramebufferCreateInfo.debugName = "Lit HDR Framebuffer";
		litHdrFramebufferCreateInfo.renderTargetLists = &litHdrImages;
		litHdrFramebufferCreateInfo.numRenderTargetLists = 1;
		litHdrFramebufferCreateInfo.depthTarget = nullptr;
		litHdrFramebufferCreateInfo.renderPass = nullptr;
		litHdrFramebuffer = core->CreateFramebuffer(litHdrFramebufferCreateInfo);

		// ========= Light Stuff =========
		UniformBufferBinding::CreateInfo lightUniformBufferBindingCi{};
		lightUniformBufferBindingCi.binding = 1;
		lightUniformBufferBindingCi.shaderLocation = "LightUbo";
		lightUniformBufferBindingCi.size = sizeof(LightmapStruct);
		lightUniformBufferBindingCi.stages = ShaderStageBit::AllGraphics;
		lightUniformBufferBinding = core->CreateUniformBufferBinding(lightUniformBufferBindingCi);

		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.binding = lightUniformBufferBinding;
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(LightmapStruct);
		lightUniformBufferObject = core->CreateUniformBuffer(lightUniformBufferObjectCi);

		LightmapStruct lightmapStruct;
		lightUniformBufferObject->UpdateBuffer(&lightmapStruct);

		VertexBufferLayout vertexLightPositionLayout({
			{
				0,
				Grindstone::GraphicsAPI::VertexFormat::Float2,
				"vertexPosition",
				false,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		});

		VertexBuffer::CreateInfo vboCi{};
		vboCi.debugName = "Light Vertex Position Buffer";
		vboCi.content = lightPositions;
		vboCi.count = sizeof(lightPositions) / (sizeof(float) * 2);
		vboCi.size = sizeof(lightPositions);
		vboCi.layout = &vertexLightPositionLayout;
		VertexBuffer* vbo = core->CreateVertexBuffer(vboCi);

		IndexBuffer::CreateInfo iboCi{};
		iboCi.debugName = "Light Index Buffer";
		iboCi.content = lightIndices;
		iboCi.count = sizeof(lightIndices) / sizeof(lightIndices[0]);
		iboCi.size = sizeof(lightIndices);
		IndexBuffer* ibo = core->CreateIndexBuffer(iboCi);

		VertexArrayObject::CreateInfo vaoCi{};
		vaoCi.debugName = "Light Vertex Array Object";
		vaoCi.vertexBufferCount = 1;
		vaoCi.vertexBuffers = &vbo;
		vaoCi.indexBuffer = ibo;
		planePostProcessVao = core->CreateVertexArrayObject(vaoCi);
		
		auto shaderManager = EngineCore::GetInstance().shaderManager;
		lightPipeline = shaderManager->LoadShader(nullptr, "../assets/coreAssets/pointLight").pipeline;
		tonemapPipeline = shaderManager->LoadShader(nullptr, "../assets/coreAssets/tonemap").pipeline;
		
		isFirst = false;
	}

	EngineUboStruct engineUboStruct;
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.model = glm::scale(glm::vec3(0.05f)) * glm::mat4(1);
	engineUboStruct.eyePos = eyePos;

	gbuffer->BindWrite(true);
	gbuffer->BindRead();

	float clearColor[4] = {0.3f, 0.6f, 0.9f, 1.f};
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	core->SetImmediateBlending(BlendMode::None);

	globalUniformBufferObject->UpdateBuffer(&engineUboStruct);
	globalUniformBufferObject->Bind();
	EngineCore::GetInstance().assetRendererManager->RenderQueue("Opaque");

	RenderLights(registry);
	PostProcess();
}
