#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.hpp"

class RenderTarget;
class DepthTarget;

class RenderPathDeferred : public RenderPath {
public:
	RenderPathDeferred();
	void render(Framebuffer *default);
	void renderLights();
private:
	void createFramebuffer();
	Framebuffer *gbuffer_;
	RenderTarget *render_targets_;
	DepthTarget *depth_target_;
};

#endif