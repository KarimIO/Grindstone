#ifndef _EBASE_H
#define _EBASE_H
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class EBase {
protected:
	glm::vec3 position;
	glm::vec3 angles;
	glm::vec3 scale;
public:
	//static void *CreateNew();

	glm::vec3 GetForward();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetAngles();
	glm::vec3 GetPosition();
	glm::vec3 GetScale();
	glm::mat4x4 GetModelMatrix();
};

#endif