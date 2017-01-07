#ifndef _C_CAMERA_H
#define _C_CAMERA_H

#include "CBase.h"

#include <glm/glm.hpp>

#define PROJECTION_ORTHOGRAPHIC false;
#define PROJECTION_PERSPECTIVE  true;

class CCamera {
private:
	float fov;
	float x, y, width, height;
	float near, far, aspectRatio;
	bool projection;
public:
	void SetSize(float x, float y, float width, float height);
	void SetAspectRatio(float ratio);

	void SetLookAt(glm::vec3 pos);
	void SetLookAt(float x, float y, float z);

	void SetProjection(bool perspective);

	void SetNear(float near);
	void SetFar(float far);

	void SetFocalLength(float focalLength);
	void SetFOV(float fov);
};

#endif