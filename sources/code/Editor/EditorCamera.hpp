#pragma once

// #include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
		class Framebuffer;
	}

	namespace Editor {
		class EditorCamera {
		public:
			EditorCamera(GraphicsAPI::Core *core);
			int GetPrimaryFramebufferAttachment();
			void Render();
			void ResizeViewport(uint32_t width, uint32_t height);
			void UpdateProjectionMatrix();
			void UpdateViewMatrix();
		private:
			GraphicsAPI::Framebuffer* framebuffer;
			GraphicsAPI::Core *core;
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec3 position;
			glm::quat rotation;
			uint32_t width;
			uint32_t height;
			float fov;
			float near = 0.1f;
			float far = 100.f;
		};
	}
}
