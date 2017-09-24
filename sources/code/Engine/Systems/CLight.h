#ifndef _C_LIGHT_H
#define _C_LIGHT_H

#include "CBase.h"
#include <Framebuffer.h>
#include <glm/glm.hpp>
#include "CubeInfo.h"
#include "UniformBuffer.h"

struct LightPointUBO {
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
};

struct LightSpotUBO {
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
	glm::vec3 direction;
	float innerAngle;
	float outerAngle;
	glm::vec3 buffer; // For DirectX
};

struct LightDirectionalUBO {
	glm::vec3 position;
	float sourceRadius;
	glm::vec3 color;
	float power;
};

class CPointLight {
public:
	bool castShadow;
	LightPointUBO lightUBOBuffer;
	UniformBuffer *lightUBO;
	Framebuffer *shadowFBO;
	uint32_t entityID;

	CPointLight(unsigned int entityID);
	CPointLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
	void SetShadow(bool);
	void Bind();
};

class CSpotLight {
public:
	bool castShadow;
	LightSpotUBO lightUBOBuffer;
	UniformBuffer *lightUBO;
	Framebuffer *shadowFBO;
	uint32_t entityID;

	CSpotLight(unsigned int entityID);
	CSpotLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa);
	void SetShadow(bool);
	void Bind();
};

class CDirectionalLight {
public:
	bool castShadow;
	LightDirectionalUBO lightUBOBuffer;
	UniformBuffer *lightUBO;
	Framebuffer *shadowFBO;
	uint32_t entityID;

	CDirectionalLight(unsigned int entityID);
	CDirectionalLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
	void SetShadow(bool);
	void Bind();
};

#endif