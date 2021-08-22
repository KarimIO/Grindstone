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
	GraphicsAPI::Core* core = Editor::Manager::GetInstance().GetEngineCore().GetGraphicsCore();
	GraphicsAPI::RenderTarget::CreateInfo renderTargetCreateInfo;
	renderTargetCreateInfo.width = 800;
	renderTargetCreateInfo.height = 600;
	renderTargetCreateInfo.format = GraphicsAPI::ColorFormat::R8G8B8;
	auto* renderTarget = core->CreateRenderTarget(&renderTargetCreateInfo, 1, false);

	GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.renderTargetLists = &renderTarget;
	framebufferCreateInfo.numRenderTargetLists = 1;
	framebuffer = core->CreateFramebuffer(framebufferCreateInfo);

	EngineCore& engineCore = Editor::Manager::GetInstance().GetEngineCore();
	renderer = engineCore.CreateRenderer();
	UpdateViewMatrix();
}

int EditorCamera::GetPrimaryFramebufferAttachment() {
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

void EditorCamera::ResizeViewport(uint32_t width, uint32_t height) {
	if (this->width == width && this->height == height) {
		return;
	}
	
	this->width = width;
	this->height = height;
	framebuffer->Resize(width, height);

	UpdateProjectionMatrix();
}

void EditorCamera::UpdateProjectionMatrix() {
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(fov, aspectRatio, near, far);
}

void EditorCamera::UpdateViewMatrix() {
	glm::vec3 origin = glm::vec3(4, 3, 3);
	glm::vec3 forward = glm::vec3(-1, -1, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);
	
	glm::vec3 target = origin + forward;

	view = glm::lookAt(origin, target, up);
}

