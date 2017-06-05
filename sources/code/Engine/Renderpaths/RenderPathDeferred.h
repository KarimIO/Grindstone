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
	Framebuffer *ssaoFBO;
	Framebuffer *postFBO;

	Texture *ssaoNoiseTex;
	Texture *cube;

	unsigned int numSkyIndices;

	void GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
	void SSAOPrepass(glm::mat4 projection, glm::mat4 view);
	void DeferredPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost);
	void DebugPass(glm::mat4 projection, glm::mat4 view);
	ShaderProgram *iblShader;
	ShaderProgram *directionalLightShader;
	ShaderProgram *spotLightShader;
	ShaderProgram *pointLightShader;
	ShaderProgram *spotLightShadowShader;
	ShaderProgram *pointLightShadowShader;
	ShaderProgram *skyShader;
	ShaderProgram *postShader;
	ShaderProgram *debugShader;
	ShaderProgram *ssaoShader;
	ShaderProgram *ssaoBlurShader;

	ShaderProgram *directionalShadowShader;

	VertexArrayObject *vaoSphere;
	VertexBufferObject *vboSphere;
	inline void BuildQuad();
	inline void BuildSphere();
	inline void SetupDeferredFBO();
	inline void CompileDirectionalShader(std::string &vsPath, std::string &vsContent);
	inline void CompileIBLShader(std::string vsPath, std::string vsContent);

	inline void CompilePointShader(std::string &vsPath, std::string &vsContent);
	inline void CompilePointShadowShader(std::string vsPath, std::string vsContent);
	inline void CompileSpotShader(std::string vsPath, std::string vsContent);
	inline void CompileSpotShadowShader(std::string vsPath, std::string vsContent);

	inline void CompileSkyShader();
	inline void CompileDebugShader(std::string vsPath, std::string vsContent);
	inline void CompileSSAO(std::string vsPath, std::string vsContent);

	inline void CompilePostShader(std::string vsPath, std::string vsContent);
	inline void BuildPostFBO();

public:
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost);
	virtual Framebuffer *GetGBuffer();
	virtual Framebuffer *GetFramebuffer();
	RenderPathDeferred(GraphicsWrapper *gw, SModel *gc, STerrain *terrainSystem);
};

#endif