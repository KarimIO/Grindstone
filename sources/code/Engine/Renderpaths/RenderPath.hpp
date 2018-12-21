#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <glm/glm.hpp>

class Framebuffer;

class RenderPath {
public:
	virtual void render(Framebuffer *gbuffer_, glm::mat4 p, glm::mat4 v, glm::vec3 eye) = 0;
};

#endif