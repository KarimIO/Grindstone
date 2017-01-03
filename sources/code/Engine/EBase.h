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

	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();
	glm::vec3 getAngles();
	glm::vec3 getPosition();
	glm::vec3 getScale();
	glm::mat4x4 getModelMatrix();
};

#endif