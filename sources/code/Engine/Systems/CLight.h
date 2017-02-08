#ifndef _C_LIGHT_H
#define _C_LIGHT_H

#include "CBase.h"
#include <Framebuffer.h>
#include <glm/glm.hpp>
#include "CubeInfo.h"

class CBaseLight : public CBase {
public:
	glm::vec3 lightColor;
	float intensity;
	bool castShadow;
	Framebuffer *fbo;
	glm::mat4 projection;
	CBaseLight(unsigned int entityID, glm::vec3 color, float strength, bool cast);
};

class CPointLight : public CBaseLight {
public:
	float lightRadius;
	CPointLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
};

class CSpotLight : public CPointLight {
public:
	float innerSpotAngle;
	float outerSpotAngle;
	CSpotLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa);
};

class CDirectionalLight : public CBaseLight {
public:
	float sunRadius;
	CDirectionalLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
};

#endif