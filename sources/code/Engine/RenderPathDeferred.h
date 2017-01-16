#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.h"
#include <Framebuffer.h>
#include <Shader.h>
#include "Terrain.h"

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	Framebuffer *fbo;
	Framebuffer *directionalLight;

	Texture *envMap;

	int numSkyIndices;

	void ShadowPass();
	void DirectionalShadowShader();

	void GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
	void DeferredPass(glm::vec3 eyePos, glm::vec2 res);
	void PostPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
	ShaderProgram *quadShader;
	ShaderProgram *skyShader;

	ShaderProgram *directionalShadowShader;
	

	VertexArrayObject *vaoQuad;
	VertexBufferObject *vboQuad;
	VertexArrayObject *vaoSphere;
	VertexBufferObject *vboSphere;

	STerrain terrain;
public:
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, glm::vec2 res);
	RenderPathDeferred(GraphicsWrapper *gw, SModel *gc);
};

#endif