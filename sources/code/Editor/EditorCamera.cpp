#include <glm/gtx/transform.hpp>

#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Plugins/GraphicsVulkan/VulkanDescriptorSet.hpp>

#include "EditorCamera.hpp"
#include "EditorManager.hpp"

using namespace Grindstone::Editor;
using namespace Grindstone;

EditorCamera::EditorCamera() {
	GraphicsAPI::Core* core = Editor::Manager::GetEngineCore().GetGraphicsCore();

	GraphicsAPI::RenderTarget::CreateInfo renderTargetCreateInfo{};
	renderTargetCreateInfo.debugName = "Editor Viewport Color Image";
	renderTargetCreateInfo.width = 800;
	renderTargetCreateInfo.height = 600;
	renderTargetCreateInfo.format = GraphicsAPI::ColorFormat::RGBA8;
	renderTargetCreateInfo.isSampled = true;
	renderTarget = core->CreateRenderTarget(&renderTargetCreateInfo, 1, false);

	/*
	GraphicsAPI::DepthTarget::CreateInfo depthTargetCreateInfo{};
	depthTargetCreateInfo.debugName = "Editor Viewport Depth Image";
	depthTargetCreateInfo.width = 800;
	depthTargetCreateInfo.height = 600;
	depthTargetCreateInfo.format = GraphicsAPI::DepthFormat::D24_STENCIL_8;
	auto* depthTarget = core->CreateDepthTarget(depthTargetCreateInfo);
	*/

	GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Editor RenderPass";
	renderPassCreateInfo.colorFormatCount = 1;
	renderPassCreateInfo.colorFormats = &renderTargetCreateInfo.format;
	renderPassCreateInfo.depthFormat = GraphicsAPI::DepthFormat::None;
	renderPass = core->CreateRenderPass(renderPassCreateInfo);
	
	GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.debugName = "Editor Framebuffer";
	framebufferCreateInfo.renderTargetLists = &renderTarget;
	framebufferCreateInfo.numRenderTargetLists = 1;
	framebufferCreateInfo.depthTarget = nullptr;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.width = 800;
	framebufferCreateInfo.height = 600;
	framebuffer = core->CreateFramebuffer(framebufferCreateInfo);

	GraphicsAPI::DescriptorSetLayout::Binding descriptorSetLayoutBinding{};
	descriptorSetLayoutBinding.bindingId = 0;
	descriptorSetLayoutBinding.type = GraphicsAPI::BindingType::RenderTexture;
	descriptorSetLayoutBinding.count = 1;
	descriptorSetLayoutBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.debugName = "Editor Viewport Descriptor Set Layout";
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.bindings = &descriptorSetLayoutBinding;
	descriptorSetLayout = core->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::Binding descriptorSetBinding{ renderTarget };

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Editor Viewport Descriptor Set";
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.bindings = &descriptorSetBinding;
	descriptorSetCreateInfo.layout = descriptorSetLayout;
	descriptorSet = core->CreateDescriptorSet(descriptorSetCreateInfo);

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	renderer = engineCore.CreateRenderer(renderPass);
	UpdateViewMatrix();
}

uint64_t EditorCamera::GetRenderOutput() {
	return (uint64_t)(static_cast<GraphicsAPI::VulkanDescriptorSet*>(descriptorSet)->GetDescriptorSet());
}

void EditorCamera::Render(GraphicsAPI::CommandBuffer* commandBuffer) {
	EngineCore& engineCore = Editor::Manager::GetInstance().GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	SceneManagement::SceneManager* sceneManager = engineCore.GetSceneManager();

	if (sceneManager == nullptr) {
		return;
	}

	if (sceneManager->scenes.size() == 0) {
		return;
	}

	SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;

	if (scene == nullptr) {
		return;
	}

	entt::registry& registry = scene->GetEntityRegistry();
	renderer->Render(
		commandBuffer,
		registry,
		projection,
		view,
		position,
		framebuffer
	);
}

void EditorCamera::RenderPlayModeCamera(GraphicsAPI::CommandBuffer* commandBuffer) {
	EngineCore& engineCore = Editor::Manager::GetInstance().GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	SceneManagement::SceneManager* sceneManager = engineCore.GetSceneManager();

	if (sceneManager == nullptr) {
		return;
	}

	if (sceneManager->scenes.size() == 0) {
		return;
	}

	SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;

	if (scene == nullptr) {
		return;
	}

	entt::registry& registry = scene->GetEntityRegistry();
	TransformComponent* transformComponent = nullptr;
	CameraComponent* cameraComponent = nullptr;
	auto view = registry.view<TransformComponent, CameraComponent>();
	view.each(
		[&](
			TransformComponent& currentTransform,
			CameraComponent& currentCamera
			) {
				transformComponent = &currentTransform;
				cameraComponent = &currentCamera;
		}
	);

	if (cameraComponent == nullptr || cameraComponent->renderer == nullptr) {
		return;
	}

	cameraComponent->aspectRatio = static_cast<float>(width) / height;
	cameraComponent->renderer->Resize(width, height);

	glm::vec3 pos = transformComponent->position;
	const glm::mat4 viewMatrix = glm::lookAt(
		pos,
		pos + transformComponent->GetForward(),
		-transformComponent->GetUp()
	);

	const glm::mat4 projectionMatrix = glm::perspective(
		cameraComponent->fieldOfView,
		cameraComponent->aspectRatio,
		cameraComponent->nearPlaneDistance,
		cameraComponent->farPlaneDistance
	);

	renderer->Render(
		commandBuffer,
		registry,
		projectionMatrix,
		viewMatrix,
		pos,
		framebuffer
	);
}

const float maxAngle = 1.55f;
void EditorCamera::OffsetRotation(float xOffset, float yOffset) {
	float deltaTime = (float)Editor::Manager::GetEngineCore().GetDeltaTime();

	const float mouseSensitivity = 30.0f;
	eulerAngles.x += yOffset * mouseSensitivity * deltaTime / width;
	eulerAngles.y -= xOffset * mouseSensitivity * deltaTime / height;

	if (eulerAngles.x < -maxAngle) {
		eulerAngles.x = -maxAngle;
	}
	else if (eulerAngles.x > maxAngle) {
		eulerAngles.x = maxAngle;
	}

	rotation = glm::quat(eulerAngles);
	UpdateViewMatrix();
}

void EditorCamera::OffsetPosition(float x, float y, float z) {
	float deltaTime = (float)Editor::Manager::GetEngineCore().GetDeltaTime();

	const float speed = 45.f;
	position += (
		GetForward() * z +
		GetRight() * x +
		GetUp() * y
	) * deltaTime * speed;

	UpdateViewMatrix();
}

glm::vec3 EditorCamera::GetForward() const {
	return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 EditorCamera::GetRight() const {
	return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 EditorCamera::GetUp() const {
	return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void EditorCamera::ResizeViewport(uint32_t width, uint32_t height) {
	if (this->width == width && this->height == height) {
		return;
	}

	this->width = width;
	this->height = height;

	if (width == 0 || height == 0)
	{
		return;
	}

	GraphicsAPI::Core* core = Editor::Manager::GetEngineCore().GetGraphicsCore();
	core->WaitUntilIdle();

	renderTarget->Resize(width, height);
	framebuffer->Resize(width, height);
	renderer->Resize(width, height);

	GraphicsAPI::DescriptorSet::Binding descriptorSetBinding{ renderTarget };

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Editor Viewport Descriptor Set";
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.bindings = &descriptorSetBinding;
	descriptorSetCreateInfo.layout = descriptorSetLayout;
	descriptorSet = core->CreateDescriptorSet(descriptorSetCreateInfo);

	UpdateProjectionMatrix();
}

void EditorCamera::UpdateProjectionMatrix() {
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

void EditorCamera::UpdateViewMatrix() {
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 target = position + GetForward();
	view = glm::lookAt(position, target, up);
}

glm::mat4& EditorCamera::GetProjectionMatrix() {
	return projection;
}

glm::mat4& EditorCamera::GetViewMatrix() {
	return view;
}

BaseRenderer* EditorCamera::GetRenderer() const {
	return renderer;
}
