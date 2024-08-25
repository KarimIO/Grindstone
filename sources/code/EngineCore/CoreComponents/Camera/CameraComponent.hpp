#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	class BaseRenderer;

	namespace GraphicsAPI {
		class RenderPass;
		class Framebuffer;
		class CommandBuffer;
	}

	namespace Events {
		struct BaseEvent;
	}

	struct CameraComponent {
		bool isMainCamera = true;
		bool isOrthographic = false;
		float nearPlaneDistance = 0.1f;
		float farPlaneDistance = 200.0f;
		float fieldOfView = glm::radians(90.0f);
		float aspectRatio = 800.0f / 600.0f;
		BaseRenderer* renderer = nullptr;
		Grindstone::GraphicsAPI::RenderPass* renderPass = nullptr;
		std::vector<Grindstone::GraphicsAPI::Framebuffer*> framebuffers;
		std::vector<Grindstone::GraphicsAPI::CommandBuffer*> commandBuffers;

		bool OnWindowResize(Events::BaseEvent* ev);

		REFLECT("Camera")
	};

	void SetupCameraComponent(entt::registry&, entt::entity);
	void DestroyCameraComponent(entt::registry&, entt::entity);
}
