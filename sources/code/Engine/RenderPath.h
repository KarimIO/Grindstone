#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <GraphicsWrapper.h>
#include "SGeometry.h"
#include <glm/glm.hpp>

class RenderPath {
public:
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, glm::vec2 res) = 0;
};

#endif