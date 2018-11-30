#ifndef _RENDERPATH_H
#define _RENDERPATH_H

class Framebuffer;

class RenderPath {
public:
	virtual void render(Framebuffer *gbuffer_) = 0;
};

#endif