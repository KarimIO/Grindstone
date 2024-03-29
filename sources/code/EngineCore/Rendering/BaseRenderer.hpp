#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
		class Framebuffer;
		class CommandBuffer;
	}

	namespace Events {
		struct BaseEvent;
	}

	class BaseRenderer {
	public:
		virtual ~BaseRenderer() {};
		virtual bool OnWindowResize(Events::BaseEvent*) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Render(
			GraphicsAPI::CommandBuffer* commandBuffer,
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer = nullptr // If nullptr, use default framebuffer
		) = 0;
	};
}
