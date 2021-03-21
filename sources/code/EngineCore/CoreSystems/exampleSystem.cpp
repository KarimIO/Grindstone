#include <iostream>
#include <entt/entt.hpp>
#include "exampleSystem.hpp"
#include "../CoreComponents/Transform/TransformComponent.hpp"

namespace Grindstone {
	void exampleSystem(entt::registry& registry) {
		auto view = registry.view<TransformComponent>();

		view.each([](auto &transformComponent) {
			transformComponent.position[1] += 0.01f;
		});
	}
}
