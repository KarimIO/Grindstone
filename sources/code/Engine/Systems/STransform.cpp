#include "STransform.hpp"
#include <glm/gtx/transform.hpp>

void STransform::AddComponent(unsigned int entityID, unsigned int &target) {
	components.push_back(CTransform());
	components.back().entityID = entityID;
	target = (unsigned int)(components.size() - 1);
}

void STransform::Update() {
	for (auto &component : components) {
		component.Update();
	}
}

CTransform::CTransform() {
	componentType = COMPONENT_TRANSFORM;
	scale = glm::vec3(1, 1, 1);
}

glm::vec3 CTransform::GetForward() {
	glm::vec3 ang = GetAngles();
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 CTransform::GetUp() {
	return glm::cross(GetRight(), GetForward());
}

glm::vec3 CTransform::GetRight() {
	glm::vec3 ang = GetAngles();
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}

glm::vec3 CTransform::GetPosition() {
	return position;
}

glm::vec3 CTransform::GetAngles() {
	return angles;
}

glm::vec3 CTransform::GetScale() {
	return scale;
}

void CTransform::Update() {
	model = glm::mat4(1);
	model = glm::translate(
		model,
		position);
	model = glm::scale(model, GetScale());
	float maxRot = (std::fmax(std::fmax(angles.x, angles.y), angles.z));
	if (maxRot > 0)
		model = glm::rotate(model, maxRot, angles / maxRot);
}

glm::mat4x4 &CTransform::GetModelMatrix() {
	return model;
}