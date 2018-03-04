#ifndef _CUBEMAP_SYSTEM_H
#define _CUBEMAP_SYSTEM_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>
#include <vector>
#include "Texture.hpp"

#include "CubeInfo.hpp"

class CubemapComponent;
class CubemapSystem {
private:
	bool writing;
	std::vector<CubemapComponent> components;
public:
	void CaptureCubemaps(double);
	void LoadCubemaps();
	void Reserve(int n);
	CubemapComponent *GetClosestCubemap(glm::vec3);
	CubemapComponent *AddCubemap(glm::vec3);
};

class CubemapComponent {
public:
	glm::vec3 position;
	std::string cubemapName;
	uint32_t cubeResolution;
	Texture *cubemap;
	TextureBinding *cubemap_binding;
};

#endif