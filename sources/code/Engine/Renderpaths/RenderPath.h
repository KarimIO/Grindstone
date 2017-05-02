#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <GraphicsWrapper.h>
#include "../Systems/SGeometry.h"
#include "Framebuffer.h"
#include <glm/glm.hpp>

class RenderPath {
public:
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) = 0;
	virtual Framebuffer *GetFramebuffer() = 0;
};

#endif