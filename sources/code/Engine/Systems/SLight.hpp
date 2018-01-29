#ifndef _S_LIGHT_H
#define _S_LIGHT_H

#include <vector>
#include "GraphicsWrapper.hpp"
#include "SGeometry.hpp"
#include "../Core/System.hpp"
#include "CBase.hpp"
#include <Framebuffer.hpp>
#include <glm/glm.hpp>
#include "CubeInfo.hpp"
#include "UniformBuffer.hpp"

struct LightPointUBO {
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
};

struct LightSpotUBO {
	glm::mat4 shadow_mat;
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
	glm::vec3 direction;
	float innerAngle;
	float outerAngle;
};

struct LightDirectionalUBO {
	glm::mat4 shadow_mat;
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
	DepthTarget *shadow_db_;
	uint32_t entityID;
	glm::mat4 matrix_;

	CSpotLight(unsigned int entityID);
	CSpotLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa);
	void SetShadow(bool);
	void Bind();
	glm::mat4 calculateMatrix();
	glm::mat4 calculateMatrixBasis();
};

class CDirectionalLight {
public:
	bool castShadow;
	LightDirectionalUBO lightUBOBuffer;
	UniformBuffer *lightUBO;
	Framebuffer *shadowFBO;
	DepthTarget *shadow_db_;
	uint32_t entityID;
	glm::mat4 matrix_;

	CDirectionalLight(unsigned int entityID);
	CDirectionalLight(unsigned int entityID, glm::vec3 color, float strength, bool cast, float radius);
	void SetShadow(bool);
	void Bind();
	glm::mat4 calculateMatrix();
	glm::mat4 calculateMatrixBasis();
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
	SGeometry *geometry_system;

	GraphicsPipeline *m_pointLightPipeline;
	GraphicsPipeline *m_spotLightPipeline;
	GraphicsPipeline *m_directionalLightPipeline;

	std::vector<CPointLight>		pointLights;
	std::vector<CSpotLight>			spotLights;
	std::vector<CDirectionalLight>	directionalLights;

	void AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius);
	void AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerRadius, float outerRadius);
	void AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius);

	void AddPointLight(unsigned int entityID);
	void AddSpotLight(unsigned int entityID);
	void AddDirectionalLight(unsigned int entityID);

	void SetPointers(GraphicsWrapper *gw, SGeometry *gc);
	void DrawShadows();
};

#endif