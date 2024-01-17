#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "CameraComponent.hpp"
#include "Common/Event/WindowEvent.hpp"
#include "Common/Window/WindowManager.hpp"

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore/Rendering/BaseRenderer.hpp"

using namespace Grindstone;

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

void Grindstone::SetupCameraComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	CameraComponent& cameraComponent = *static_cast<CameraComponent*>(componentPtr);

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
