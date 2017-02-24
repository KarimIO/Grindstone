#ifndef _C_POSITIONAL_H
#define _C_POSITIONAL_H

#include "CBase.h"
#include "glm/glm.hpp"

class CPositional : public CBase {
public:
	glm::vec3 position;
	glm::vec3 angles;
	glm::vec3 scale;
};

class SPositional {
public:
};

#endif