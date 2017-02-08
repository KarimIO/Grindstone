#ifndef _S_LIGHT_H
#define _S_LIGHT_H

#include "CLight.h"
#include <vector>
#include "GraphicsWrapper.h"
#include "SGeometry.h"
#include "../Core/System.h"

class LightComponentList : public SpaceComponentList {
public:
};

class SLight : public System {
public:
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	std::vector<CPointLight>		pointLights;
	std::vector<CSpotLight>			spotLights;
	std::vector<CDirectionalLight>	directionalLights;

	void AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius);
	void AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerRadius, float outerRadius);
	void AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius);

	void SetPointers(GraphicsWrapper *gw, SModel *gc);
	void DrawShadows();
};

#endif