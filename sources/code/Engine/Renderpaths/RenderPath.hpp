#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <Framebuffer.hpp>

class RenderPath {
public:
	virtual void Render(Framebuffer *) = 0;
};

#endif