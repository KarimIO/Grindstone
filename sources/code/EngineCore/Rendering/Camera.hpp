#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Grindstone {
	class Camera {
	public:
		void Render();
		void ResizeViewport(uint32_t width, uint32_t height);
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();
	private:
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 position;
		glm::quat rotation;
		uint32_t width;
		uint32_t height;
		float fieldOfView;
		float nearPlaneDistance = 0.1f;
		float farPlaneDistance = 200.f;
	};
}
