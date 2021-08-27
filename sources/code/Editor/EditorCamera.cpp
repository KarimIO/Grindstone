#include <glm/gtx/transform.hpp>
#include "EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Framebuffer.hpp"
#include "Common/Graphics/Core.hpp"
#include "EditorCamera.hpp"
#include "EngineCore/Rendering/BaseRenderer.hpp"
#include "EngineCore/Scenes/Manager.hpp"
using namespace Grindstone::Editor;
using namespace Grindstone;

EditorCamera::EditorCamera() {
	GraphicsAPI::Core* core = Editor::Manager::GetEngineCore().GetGraphicsCore();
	GraphicsAPI::RenderTarget::CreateInfo renderTargetCreateInfo;
	renderTargetCreateInfo.width = 800;
	renderTargetCreateInfo.height = 600;
	renderTargetCreateInfo.format = GraphicsAPI::ColorFormat::R8G8B8;
	auto* renderTarget = core->CreateRenderTarget(&renderTargetCreateInfo, 1, false);

	GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.renderTargetLists = &renderTarget;
	framebufferCreateInfo.numRenderTargetLists = 1;
	framebuffer = core->CreateFramebuffer(framebufferCreateInfo);

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	renderer = engineCore.CreateRenderer();
	UpdateViewMatrix();
}

uint32_t EditorCamera::GetPrimaryFramebufferAttachment() {
	return framebuffer->GetAttachment(0);
}

void EditorCamera::Render() {
	EngineCore& engineCore = Editor::Manager::GetInstance().GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto sceneManager = engineCore.GetSceneManager();
	auto scene = sceneManager->scenes.begin()->second;
	auto& registry = scene->GetEntityRegistry();

	renderer->Render(
		registry,
		projection,
		view,
		position,
		framebuffer
	);

	framebuffer->Unbind();
}

const float maxAngle = 1.55f;
void EditorCamera::OffsetRotation(float pitch, float yaw) {
	float deltaTime = (float)Editor::Manager::GetEngineCore().GetDeltaTime();

	const float mouseSensitivity = 1.f;
	eulerAngles.x += pitch * mouseSensitivity * deltaTime;
	eulerAngles.y -= yaw * mouseSensitivity * deltaTime;

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

glm::vec3 EditorCamera::GetForward() {
	return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 EditorCamera::GetRight() {
	return rotation * glm::vec3(-1.0f, 0.0f, 0.0f);
}

glm::vec3 EditorCamera::GetUp() {
	return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}


void EditorCamera::ResizeViewport(uint32_t width, uint32_t height) {
	if (this->width == width && this->height == height) {
		return;
	}
	
	this->width = width;
	this->height = height;
	framebuffer->Resize(width, height);
	renderer->Resize(width, height);

	UpdateProjectionMatrix();
}

void EditorCamera::UpdateProjectionMatrix() {
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(fov, aspectRatio, near, far);
}

void EditorCamera::UpdateViewMatrix() {
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 target = position + GetForward();
	view = glm::lookAt(position, target, up);
}

