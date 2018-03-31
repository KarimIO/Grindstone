#ifndef _C_TRANSFORM_H
#define _C_TRANSFORM_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "CBase.hpp"
#include <vector>

class CTransform : public CBase {
public:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 angles;
	glm::vec3 scale;
	glm::mat4 model;
public:
	CTransform();
	void Update();
	glm::vec3 GetForward();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetAngles();
	glm::vec3 GetPosition();
	glm::vec3 GetVelocity();
	glm::vec3 GetScale();
	glm::mat4x4 &GetModelMatrix();
};

class STransform {
public:
	void AddComponent(unsigned int entityID, unsigned int &target);
	void Update();
	std::vector<CTransform> components;
};

#endif