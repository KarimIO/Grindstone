#ifndef _S_LIGHT_H
#define _S_LIGHT_H

#include "CLight.h"
#include <vector>
#include "GraphicsWrapper.h"
#include "SGeometry.h"
#include "../Core/System.h"

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
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;

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