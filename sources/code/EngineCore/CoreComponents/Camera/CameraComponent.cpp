#include <Common/Event/WindowEvent.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/CommandBuffer.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "CameraComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

REFLECT_STRUCT_BEGIN(CameraComponent)
	REFLECT_STRUCT_MEMBER(isMainCamera)
	REFLECT_STRUCT_MEMBER(isOrthographic)
	REFLECT_STRUCT_MEMBER(nearPlaneDistance)
	REFLECT_STRUCT_MEMBER(farPlaneDistance)
	REFLECT_STRUCT_MEMBER(fieldOfView)
	REFLECT_STRUCT_MEMBER(aspectRatio)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

bool CameraComponent::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() != Events::EventType::WindowResize) {
		return false;
	}

	Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
	aspectRatio = (float)winResizeEvent->width / (float)winResizeEvent->height;
	renderer->Resize(winResizeEvent->width, winResizeEvent->height);
	return true;
}

void Grindstone::SetupCameraComponent(entt::registry& registry, entt::entity entity) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::Window* window = engineCore.windowManager->GetWindowByIndex(0);
	Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb = window->GetWindowGraphicsBinding();
	Grindstone::Events::Dispatcher* eventDispatcher = engineCore.GetEventDispatcher();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	if (engineCore.isEditor) {
		return;
	}

	CameraComponent& cameraComponent = registry.get<CameraComponent>(entity);
	cameraComponent.renderer = engineCore.CreateRenderer(cameraComponent.renderPass);

	eventDispatcher->AddEventListener(
		Events::EventType::WindowResize,
		std::bind(&CameraComponent::OnWindowResize, &cameraComponent, std::placeholders::_1)
	);

	Grindstone::GraphicsAPI::ColorFormat colorFormat = wgb->GetSwapchainFormat();
	Grindstone::GraphicsAPI::RenderPass::AttachmentInfo colorAttachment{ colorFormat, false };

	Grindstone::GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Camera Renderpass";
	renderPassCreateInfo.colorAttachments = &colorAttachment;
	renderPassCreateInfo.colorAttachmentCount = 1;
	renderPassCreateInfo.depthFormat = Grindstone::GraphicsAPI::DepthFormat::None;
	renderPassCreateInfo.shouldClearDepthOnLoad = false;
	cameraComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	uint32_t imageCount = wgb->GetMaxFramesInFlight();
	cameraComponent.commandBuffers.resize(imageCount);
	GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};
	GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.depthTarget = nullptr;
	framebufferCreateInfo.isCubemap = false;
	framebufferCreateInfo.numRenderTargetLists = 1;
	framebufferCreateInfo.renderPass = cameraComponent.renderPass;

	unsigned int width, height;
	window->GetWindowSize(width, height);
	framebufferCreateInfo.width = width;
	framebufferCreateInfo.height = height;
	for (uint32_t i = 0; i < imageCount; ++i) {
		Grindstone::GraphicsAPI::RenderTarget* renderTarget = wgb->GetSwapchainRenderTarget(i);
		{
			std::string debugName = "Camera Framebuffer " + std::to_string(i);
			framebufferCreateInfo.debugName = debugName.c_str();
			framebufferCreateInfo.renderTargetLists = &renderTarget;
			cameraComponent.framebuffers[i] = graphicsCore->CreateFramebuffer(framebufferCreateInfo);
		}

		{
			std::string debugName = "Camera Command Buffer " + std::to_string(i);
			commandBufferCreateInfo.debugName = debugName.c_str();
			cameraComponent.commandBuffers[i] = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
		}
	}
}

void Grindstone::DestroyCameraComponent(entt::registry& registry, entt::entity entity) {
	CameraComponent& cameraComponent = registry.get<CameraComponent>(entity);
	AllocatorCore::Free(cameraComponent.renderer);
}
