#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Framebuffer;
	}

	class BaseRenderer {
	public:
		virtual ~BaseRenderer() {};
		virtual void Render(
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer = nullptr // If nullptr, use default framebuffer
		) = 0;
	};
}
