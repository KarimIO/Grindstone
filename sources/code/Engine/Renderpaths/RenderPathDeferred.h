#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.h"
#include "../Systems/Terrain.h"
#include <Framebuffer.h>
#include <Shader.h>
#include "../Systems/Terrain.h"

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
	STerrain *terrainSystem;
	Framebuffer *fbo;
	Framebuffer *postFBO;

	unsigned int numSkyIndices;

	void GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
	void DeferredPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost);
	void PostPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
	ShaderProgram *iblShader;
	ShaderProgram *directionalLightShader;
	ShaderProgram *spotLightShader;
	ShaderProgram *pointLightShader;
	ShaderProgram *skyShader;
	ShaderProgram *postShader;
	ShaderProgram *debugShader;

	ShaderProgram *directionalShadowShader;

	VertexArrayObject *vaoQuad;
	VertexBufferObject *vboQuad;
	VertexArrayObject *vaoSphere;
	VertexBufferObject *vboSphere;
public:
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost);
	RenderPathDeferred(GraphicsWrapper *gw, SModel *gc, STerrain *terrainSystem);
};

#endif