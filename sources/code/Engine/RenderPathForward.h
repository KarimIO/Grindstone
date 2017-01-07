#ifndef _RENDERPATH_FORWARD_H
#define _RENDERPATH_FORWARD_H

#include "RenderPath.h"

class RenderPathForward : public RenderPath {
	float *quadVertices;
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	void PrePass();
	void GeometryPass();
	void PostPass();
public:
	RenderPathForward(GraphicsWrapper *gw, SModel *gc);
	virtual void Draw(glm::vec3 eyePos, glm::vec2 res);
};

#endif