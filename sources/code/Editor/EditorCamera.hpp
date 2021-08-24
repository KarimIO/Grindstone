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
			void OffsetRotation(float pitch, float yaw);
			void OffsetPosition(float x, float y, float z);
			void ResizeViewport(uint32_t width, uint32_t height);
			void UpdateProjectionMatrix();
			void UpdateViewMatrix();
		private:
			glm::vec3 GetForward();
			glm::vec3 GetRight();
			glm::vec3 GetUp();

			GraphicsAPI::Framebuffer* framebuffer = nullptr;
			BaseRenderer* renderer = nullptr;
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec3 position;
			glm::vec3 eulerAngles;
			glm::quat rotation;
			uint32_t width = 800;
			uint32_t height = 600;
			float fov = glm::radians(80.0f);
			float near = 0.1f;
			float far = 100.f;
		};
	}
}
