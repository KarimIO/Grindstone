#include "CCamera.h"
#include "../Core/Engine.h"

void CCamera::SetSize(float X, float Y, float Width, float Height) {
	x = X;
	y = Y;
	width = Width;
	height = Height;
}

void CCamera::SetAspectRatio(float ratio) {
	aspectRatio = ratio;
}

void CCamera::SetLookAt(glm::vec3 pos) {
}

void CCamera::SetLookAt(float x, float y, float z) {
}

void CCamera::SetProjection(bool perspective) {
}

void CCamera::SetNear(float Near) {
	camNear = Near;
}

void CCamera::SetFar(float Far) {
	camFar = Far;
}

void CCamera::SetFocalLength(float focalLength) {
	
}

void CCamera::SetFOV(float fov) {

}

glm::mat4 CCamera::GetProjection() {
	glm::mat4 projection = glm::perspective(engine.settings.fov, aspectRatio, 0.1f, 100.0f);

	return projection;
}

glm::mat4 CCamera::GetView() {
	CTransform *transform = &engine.transformSystem.components[engine.entities[entityID].components[COMPONENT_TRANSFORM]];
	glm::mat4 view = glm::lookAt(
		transform->GetPosition(),
		transform->GetPosition() + transform->GetForward(),
		transform->GetUp()
	);

	return view;
}

void SCamera::AddComponent(unsigned int entID, unsigned int & target) {
	components.push_back(CCamera());
	components[components.size() - 1].entityID = entID;
	target = (unsigned int)(components.size() - 1);
}
