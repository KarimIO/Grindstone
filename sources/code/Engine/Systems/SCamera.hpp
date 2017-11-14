#ifndef _C_CAMERA_H
#define _C_CAMERA_H

#include "CBase.h"
#include <vector>
#include "CTransform.h"

#include <glm/glm.hpp>
#include "Framebuffer.h"
#include "GraphicsPipeline.h"

#define PROJECTION_ORTHOGRAPHIC false;
#define PROJECTION_PERSPECTIVE  true;

class CCamera : public CBase {
private:
	float fov;
	bool automatic;
	float aperture, iso, shutterSpeed;
	float x, y, width, height;
	float camNear, camFar, aspectRatio;
	bool projection;

	Framebuffer *fbo;
	GraphicsPipeline *shader;

	float CalculateExposure(float middleVal = 0.18f);
public:
	CCamera();
	void SetSize(float x, float y, float width, float height);
	void SetAspectRatio(float ratio);

	void SetLookAt(glm::vec3 pos);
	void SetLookAt(float x, float y, float z);

	void SetProjection(bool perspective);

	void SetNear(float near);
	void SetFar(float far);
	
	void SetShutterSpeed(float _speed);
	void SetApertureSize(float _aperture);
	void SetISO(float _iso);
	void SetFOV(float _fov);

	void PostProcessing(Framebuffer *fbo);
	Framebuffer *GetFramebuffer();

	glm::mat4 GetProjection();
	glm::mat4 GetView();
};

class SCamera {
public:
	std::vector<CCamera> components;
	void AddComponent(unsigned int entID, unsigned int &target);
};

#endif