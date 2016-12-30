#include "EBase.h"
#include <glm/gtx/transform.hpp>

glm::vec3 EBase::getForward() {
	glm::vec3 ang = getAngles();
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 EBase::getUp() {
	return glm::cross(getRight(), getForward());
}

glm::vec3 EBase::getRight() {
	glm::vec3 ang = getAngles();
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}

glm::vec3 EBase::getPosition() {
	return position;
}

glm::vec3 EBase::getAngles() {
	return angles;
}

glm::vec3 EBase::getScale() {
	return scale;
}

glm::mat4x4 EBase::getModelMatrix() {
	glm::mat4x4 Model = glm::mat4(1);
	Model = glm::translate(
		Model,
		position);
	Model = glm::scale(Model, getScale());
	float maxRot = (glm::max(glm::max(angles.x, angles.y), angles.z));
	if (maxRot > 0)
		Model = glm::rotate(Model, maxRot, angles / maxRot);

	return Model;
}