#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <Framebuffer.hpp>

class RenderPath {
public:
	virtual void Draw(Framebuffer *) = 0;
};

#endif