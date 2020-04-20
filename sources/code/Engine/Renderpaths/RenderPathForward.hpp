#ifndef _RENDERPATH_FORWARD_H
#define _RENDERPATH_FORWARD_H

#include "RenderPath.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>

class RenderPathForward : public RenderPath {
public:
	RenderPathForward(Grindstone::GraphicsAPI::GraphicsWrapper *gw) {}
	virtual void Draw(Grindstone::GraphicsAPI::Framebuffer *) {}
	virtual void recreateFramebuffer(unsigned int w, unsigned int h) {}
};

#endif