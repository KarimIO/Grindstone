#ifndef _S_LIGHT_H
#define _S_LIGHT_H

#include "CLight.h"
#include <vector>
#include "GraphicsWrapper.h"
#include "SGeometry.h"

class SLight {
public:
	std::vector<CPointLight>		pointLights;
	std::vector<CSpotLight>			spotLights;
	std::vector<CDirectionalLight>	directionalLights;

	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;

	void SetPointers(GraphicsWrapper *gw, SModel *gc);
	void AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius);
	void AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerRadius, float outerRadius);
	void AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius);
	void DrawShadows();
};

#endif