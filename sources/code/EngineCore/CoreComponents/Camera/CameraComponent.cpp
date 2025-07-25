#include <Common/Event/WindowEvent.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

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

void Grindstone::SetupCameraComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	CameraComponent& cameraComponent = cxtSet.GetEntityRegistry().get<CameraComponent>(entity);

	GraphicsAPI::RenderPass* renderPass = wgb->GetRenderPass();
	if (renderPass != nullptr) {
		cameraComponent.renderer = engineCore.GetRendererFactory()->CreateRenderer(wgb->GetRenderPass());

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

void Grindstone::DestroyCameraComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	CameraComponent& cameraComponent = cxtSet.GetEntityRegistry().get<CameraComponent>(entity);
	AllocatorCore::Free(cameraComponent.renderer);
}
