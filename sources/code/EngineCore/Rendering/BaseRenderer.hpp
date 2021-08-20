#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
	}

	void BaseRender(
		entt::registry& registry,
		glm::mat4 projectionMatrix,
		glm::mat4 viewMatrix,
		glm::vec3 eyePos
	);
}
