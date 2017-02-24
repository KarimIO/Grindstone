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
	CBaseLight(unsigned int entityID);
	CBaseLight(unsigned int entityID, glm::vec3 color, float strength, bool cast);
};

class CPointLight : public CBaseLight {
public:
	float lightRadius;
	CPointLight(unsigned int entityID);
	CPointLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
	void SetShadow(bool);
};

class CSpotLight : public CBaseLight {
public:
	float lightRadius;
	float innerSpotAngle;
	float outerSpotAngle;
	CSpotLight(unsigned int entityID);
	CSpotLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa);
	void SetShadow(bool);
};

class CDirectionalLight : public CBaseLight {
public:
	float sunRadius;
	CDirectionalLight(unsigned int entityID);
	CDirectionalLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
	void SetShadow(bool);
};

#endif