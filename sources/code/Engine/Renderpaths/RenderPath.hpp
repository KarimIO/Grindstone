#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <glm/glm.hpp>

class Framebuffer;
class Space;

class RenderPath {
public:
	virtual void setDebugMode(unsigned int d) = 0;
	virtual void render(Framebuffer *gbuffer_, Space *scene) = 0;
};

#endif