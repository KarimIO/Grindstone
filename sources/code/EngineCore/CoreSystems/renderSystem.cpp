#include <iostream>
#include <entt/entt.hpp>
#include "renderSystem.hpp"
#include "../CoreComponents/Camera/CameraComponent.hpp"

namespace Grindstone {
	void renderSystem(entt::registry& registry) {
		auto view = registry.view<CameraComponent>();

		view.each([](CameraComponent& cameraComponent) {
		});
	}
}
