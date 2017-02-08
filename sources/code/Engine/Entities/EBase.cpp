#include "EBase.h"
#include "../Core/Engine.h"
#include <glm/gtx/transform.hpp>

LINK_ENTITY_TO_CLASS("BaseEntity", EBase)

void EBase::Spawn() {
}

EBase::EBase() {
	id = engine.entities.size();
	scale = glm::vec3(1, 1, 1);
}

glm::vec3 EBase::GetForward() {
	glm::vec3 ang = GetAngles();
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 EBase::GetUp() {
	return glm::cross(GetRight(), GetForward());
}

glm::vec3 EBase::GetRight() {
	glm::vec3 ang = GetAngles();
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}

glm::vec3 EBase::GetPosition() {
	return position;
}

glm::vec3 EBase::GetAngles() {
	return angles;
}

glm::vec3 EBase::GetScale() {
	return scale;
}

glm::mat4x4 EBase::GetModelMatrix() {
	glm::mat4x4 Model = glm::mat4(1);
	Model = glm::translate(
		Model,
		position);
	Model = glm::scale(Model, GetScale());
	float maxRot = (max(max(angles.x, angles.y), angles.z));
	if (maxRot > 0)
		Model = glm::rotate(Model, maxRot, angles / maxRot);

	return Model;
}