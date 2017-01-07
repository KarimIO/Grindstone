#include "CCamera.h"

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
	near = Near;
}

void CCamera::SetFar(float Far) {
	far = Far;
}

void CCamera::SetFocalLength(float focalLength) {
	
}

void CCamera::SetFOV(float fov) {
	
}
