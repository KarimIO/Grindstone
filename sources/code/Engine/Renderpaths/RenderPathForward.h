#ifndef _RENDERPATH_FORWARD_H
#define _RENDERPATH_FORWARD_H

#include "RenderPath.h"
#include <GraphicsWrapper.h>

class RenderPathForward : public RenderPath {
public:
	RenderPathForward(GraphicsWrapper *gw);
	virtual void Draw(Framebuffer *);
};

#endif