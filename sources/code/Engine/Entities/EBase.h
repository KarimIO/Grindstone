#ifndef _EBASE_H
#define _EBASE_H
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "../Systems/CBase.h"

#define DECLARE_ENTITY static void CreateNew

class EBase {
protected:
	size_t id;
public:
	DECLARE_ENTITY();
	virtual void Spawn();
	glm::vec3 position;
	glm::vec3 angles;
	glm::vec3 scale;
	EBase();
	unsigned int components[NUM_COMPONENTS];
	glm::vec3 GetForward();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetAngles();
	glm::vec3 GetPosition();
	glm::vec3 GetScale();
	glm::mat4x4 GetModelMatrix();
};

#endif