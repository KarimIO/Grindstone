#include <glm/gtx/transform.hpp>
#include "Common/Graphics/Core.hpp"
#include "Camera.hpp"
using namespace Grindstone;

void Camera::Render() {
	GraphicsAPI::Core *core = nullptr;
}

void Camera::ResizeViewport(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	UpdateProjectionMatrix();
}

void Camera::UpdateProjectionMatrix() {
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

void Camera::UpdateViewMatrix() {
	glm::vec3 origin = glm::vec3(4, 3, 3);
	glm::vec3 forward = glm::vec3(-1, -1, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);
	
	glm::vec3 target = origin + forward;

	view = glm::lookAt(origin, target, up);
}
