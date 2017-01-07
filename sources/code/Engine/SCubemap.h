#ifndef _CUBEMAP_SYSTEM_H
#define _CUBEMAP_SYSTEM_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "BaseClass.h"
#include <string>
#include <vector>
#include "Texture.h"

struct CubemapDirection
{
	char name[3];
	glm::vec3 Target;
	glm::vec3 Up;
};

class CubemapComponent;
class CubemapSystem {
private:
	bool writing;
	std::vector<CubemapComponent> components;
public:
	void CaptureCubemaps(double);
	void LoadCubemaps();
	CubemapComponent *GetClosestCubemap(glm::vec3);
	CubemapComponent *AddCubemap(glm::vec3);
};

class CubemapComponent {
public:
	glm::vec3 position;
	std::string cubemapName;
	uint32_t cubeResolution;
	Texture *cubemap;
};

#endif