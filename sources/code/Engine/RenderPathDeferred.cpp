#include "RenderPathDeferred.h"
#include "GraphicsDLLPointer.h"
#include "Utilities.h"
#include "gl3w.h"
#include "Engine.h"

#include "TextureManager.h"

struct UniformBufferDef {
	glm::vec3 eyePos;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
	int texLoc4;
} defUBO;

void RenderPathDeferred::GeometryPass() {
	// Uses screen resolution due to framebuffer size
	engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
	fbo->WriteBind();
	graphicsWrapper->Clear(CLEAR_ALL);
	geometryCache->Draw();
}

void RenderPathDeferred::DeferredPass(glm::vec3 eyePos, glm::vec2 res) {
	engine.graphicsWrapper->SetResolution(0, 0, res.x, res.y);
	defUBO.eyePos = eyePos;
	defUBO.texLoc0 = 0;
	defUBO.texLoc1 = 1;
	defUBO.texLoc2 = 2;
	defUBO.texLoc3 = 3;
	defUBO.texLoc4 = 4;

	fbo->Unbind();
	shader->Use();

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	fbo->BindTexture(3);
	//envMap->BindCubemap(4);
	CubemapComponent *comp = engine.cubemapSystem.GetClosestCubemap(eyePos);
	if (comp != NULL) {
		Texture *cube = engine.cubemapSystem.GetClosestCubemap(eyePos)->cubemap;
		if (cube != NULL)
			cube->BindCubemap(4);
	}

	shader->PassData(&defUBO);
	shader->SetVec3();
	shader->SetInteger();
	shader->SetInteger();
	shader->SetInteger();
	shader->SetInteger();
	shader->SetInteger();

	graphicsWrapper->Clear(CLEAR_COLOR);

	vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	vaoQuad->Unbind();
}

void RenderPathDeferred::PostPass() {
}

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * gw, SModel * gc) {
	float tempVerts[8] = {
		-1,-1,
		1,-1,
		-1, 1,
		1, 1,
	};

	graphicsWrapper = gw;
	geometryCache = gc;

	vaoQuad = pfnCreateVAO();
	vaoQuad->Initialize();
	vaoQuad->Bind();

	vboQuad = pfnCreateVBO();
	vboQuad->Initialize(1);
	vboQuad->AddVBO(tempVerts, sizeof(tempVerts), 2, SIZE_FLOAT, DRAW_STATIC);
	vboQuad->Bind(0);
	vaoQuad->Unbind();

	glm::vec2 res = glm::vec2(engine.settings.resolutionX, engine.settings.resolutionY);
	fbo = pfnCreateFramebuffer();
	fbo->Initialize(4);
	fbo->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, res.x, res.y);
	fbo->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, res.x, res.y);
	fbo->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, res.x, res.y);
	fbo->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, res.x, res.y);
	// Depth Buffer Issue:
	fbo->AddDepthBuffer(res.x, res.y);
	fbo->Generate();

	std::string vsPath = "../shaders/deferred.glvs";
	std::string fsPath = "../shaders/deferred.glfs";

	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed To Read File.\n");

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed To Read File.\n");

	shader = pfnCreateShader();
	shader->AddShader(&vsPath, &vsContent, SHADER_VERTEX);
	shader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT);
	shader->Compile();

	shader->SetNumUniforms(6);
	shader->CreateUniform("eyePos");
	shader->CreateUniform("texPos");
	shader->CreateUniform("texNormal");
	shader->CreateUniform("texAlbedo");
	shader->CreateUniform("texSpecular");
	shader->CreateUniform("texRefl");

	envMap = LoadCubemap("../materials/skybox/Cliff", ".tga", COLOR_SRGB);
}

void RenderPathDeferred::Draw(glm::vec3 eyePos, glm::vec2 res) {
	GeometryPass();
	DeferredPass(eyePos, res);
	PostPass();
}
