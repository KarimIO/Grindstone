#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.h"
#include <Framebuffer.h>
#include <Shader.h>

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	Framebuffer *fbo;

	Texture *envMap;

	void GeometryPass();
	void DeferredPass(glm::vec3 eyePos, glm::vec2 res);
	void PostPass();
	ShaderProgram *shader;
	VertexArrayObject *vaoQuad;
	VertexBufferObject *vboQuad;
public:
	virtual void Draw(glm::vec3 eyePos, glm::vec2 res);
	RenderPathDeferred(GraphicsWrapper *gw, SModel *gc);
};

#endif