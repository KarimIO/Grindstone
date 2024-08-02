#include <Common/Event/WindowEvent.hpp>
#include <Common/Window/WindowManager.hpp>
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
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		aspectRatio = (float)winResizeEvent->width / (float)winResizeEvent->height;
	}

	return false;
}

void Grindstone::SetupCameraComponent(entt::registry& registry, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	CameraComponent& cameraComponent = registry.get<CameraComponent>(entity);

	GraphicsAPI::RenderPass* renderPass = wgb->GetRenderPass();
	if (renderPass != nullptr) {
		cameraComponent.renderer = engineCore.CreateRenderer(wgb->GetRenderPass());

		eventDispatcher->AddEventListener(
			Events::EventType::WindowResize,
			std::bind(&BaseRenderer::OnWindowResize, cameraComponent.renderer, std::placeholders::_1)
		);
	}

	eventDispatcher->AddEventListener(
		Events::EventType::WindowResize,
		std::bind(&CameraComponent::OnWindowResize, &cameraComponent, std::placeholders::_1)
	);
}

void Grindstone::DestroyCameraComponent(entt::registry& registry, entt::entity entity) {
	CameraComponent& cameraComponent = registry.get<CameraComponent>(entity);
	AllocatorCore::Free(cameraComponent.renderer);
}
