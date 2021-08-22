#pragma once

// #include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Grindstone {
	class BaseRenderer;

	namespace GraphicsAPI {
		class Framebuffer;
	}

	namespace Editor {
		class EditorCamera {
		public:
			EditorCamera();
			int GetPrimaryFramebufferAttachment();
			void Render();
			void ResizeViewport(uint32_t width, uint32_t height);
			void UpdateProjectionMatrix();
			void UpdateViewMatrix();
		private:
			GraphicsAPI::Framebuffer* framebuffer = nullptr;
			BaseRenderer* renderer = nullptr;
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec3 position;
			glm::quat rotation;
			uint32_t width = 800;
			uint32_t height = 600;
			float fov = 90.0f;
			float near = 0.1f;
			float far = 100.f;
		};
	}
}
