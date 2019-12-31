#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Framebuffer;
		class DepthTarget;
	}
}

class Space;

class RenderPath {
public:
	virtual unsigned int getDebugMode() = 0;
	virtual void setDebugMode(unsigned int d) = 0;
	virtual void render(Grindstone::GraphicsAPI::Framebuffer *gbuffer_, Grindstone::GraphicsAPI::DepthTarget *depthTarget, Space *scene) = 0;
	virtual void recreateFramebuffer(unsigned int w, unsigned int h) = 0;
	virtual void destroyGraphics() = 0;
	virtual void reloadGraphics() = 0;
};

#endif