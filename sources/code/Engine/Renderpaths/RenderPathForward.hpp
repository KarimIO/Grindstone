#ifndef _RENDERPATH_FORWARD_H
#define _RENDERPATH_FORWARD_H

#include "RenderPath.hpp"
#include <GraphicsWrapper.hpp>

class RenderPathForward : public RenderPath {
public:
	RenderPathForward(GraphicsWrapper *gw);
	virtual void Draw(Framebuffer *);
};

#endif