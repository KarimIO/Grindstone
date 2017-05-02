#ifndef _RENDERPATH_FORWARD_H
#define _RENDERPATH_FORWARD_H

#include "RenderPath.h"

class RenderPathForward : public RenderPath {
	float *quadVertices;
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	void PrePass();
	void GeometryPass(glm::mat4 projection, glm::mat4 view);
	void PostPass();
public:
	RenderPathForward(GraphicsWrapper *gw, SModel *gc);
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost);
	virtual Framebuffer *GetFramebuffer();
};

#endif