#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <Framebuffer.h>

class RenderPath {
public:
	virtual void Draw(Framebuffer *) = 0;
};

#endif