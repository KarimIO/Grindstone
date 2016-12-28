#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.h"
#include <Framebuffer.h>
#include <Shader.h>

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	Framebuffer *fbo;

	void GeometryPass();
	void DeferredPass();
	void PostPass();
	ShaderProgram *shader;
	VertexArrayObject *vaoQuad;
	VertexBufferObject *vboQuad;
public:
	virtual void Draw();
	RenderPathDeferred(GraphicsWrapper *gw, SModel *gc);
};

#endif