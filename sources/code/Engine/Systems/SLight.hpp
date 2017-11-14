#ifndef _S_LIGHT_H
#define _S_LIGHT_H

#include <vector>
#include "GraphicsWrapper.h"
#include "SGeometry.h"
#include "../Core/System.h"
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
	// For DirectX: We need to make sure the UBO is a multiple of 4
	glm::vec3 buffer;
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

struct DefferedUBO {
	glm::mat4 view;
	glm::mat4 invProj;
	glm::vec4 eyePos;
	glm::vec4 resolution;
};


class LightComponentList : public SpaceComponentList {
public:
};

class SLight : public System {
public:
	GraphicsWrapper *graphics_wrapper_;
	SModel *geometry_system;

	GraphicsPipeline *m_pointLightPipeline;
	GraphicsPipeline *m_spotLightPipeline;

	std::vector<CPointLight>		pointLights;
	std::vector<CSpotLight>			spotLights;
	std::vector<CDirectionalLight>	directionalLights;

	void AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius);
	void AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerRadius, float outerRadius);
	void AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius);

	void AddPointLight(unsigned int entityID);
	void AddSpotLight(unsigned int entityID);
	void AddDirectionalLight(unsigned int entityID);

	void SetPointers(GraphicsWrapper *gw, SModel *gc);
	void DrawShadows();
};

#endif