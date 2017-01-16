#include "RenderPathDeferred.h"
#include "GraphicsDLLPointer.h"
#include "Utilities.h"
#include "gl3w.h"
#include "Engine.h"

#include "TextureManager.h"

#include <glm/gtx/transform.hpp>

struct UniformBufferDef {
	float time;
	glm::vec3 eyePos;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
	int texLoc4;
	int sunShadow;
	glm::mat4 directionalShadowMatrix;
} defUBO;

struct SkyUniformBufferDef {
	glm::mat4 gWVP;
	float time;
} skydefUBO;

void RenderPathDeferred::ShadowPass() {
	DirectionalShadowShader();
}

void RenderPathDeferred::DirectionalShadowShader() {
	float t = (float)engine.GetTimeCurrent() / 4.0f;
	glm::vec3 pos = glm::vec3(0, sin(t), cos(t))*20.0f;
	glm::mat4 proj = glm::ortho<float>(-64, 64, -64, 64, -8, 64);
	glm::mat4 view = glm::lookAt(
		pos,
		glm::vec3(0, 0, 0),
		glm::cross(pos, glm::vec3(1,0,0))
	);

	engine.graphicsWrapper->SetResolution(0, 0, 1024, 1024);
	directionalLight->WriteBind();
	graphicsWrapper->SetDepth(1);
	graphicsWrapper->SetCull(CULL_FRONT);
	graphicsWrapper->Clear(CLEAR_ALL);
	geometryCache->Draw(proj, view);
	directionalLight->Unbind();
}

void RenderPathDeferred::GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	// Uses screen resolution due to framebuffer size
	engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
	fbo->WriteBind();
	graphicsWrapper->SetDepth(1);
	graphicsWrapper->SetCull(CULL_BACK);
	graphicsWrapper->Clear(CLEAR_ALL);
	geometryCache->Draw(projection, view);
	//terrain.Draw(projection, view, eyePos);
	fbo->Unbind();
}

void RenderPathDeferred::DeferredPass(glm::vec3 eyePos, glm::vec2 res) {
	engine.graphicsWrapper->SetResolution(0, 0, res.x, res.y);
	defUBO.time = (float)engine.GetTimeCurrent();
	defUBO.eyePos = eyePos;
	defUBO.texLoc0 = 0;
	defUBO.texLoc1 = 1;
	defUBO.texLoc2 = 2;
	defUBO.texLoc3 = 3;
	defUBO.texLoc4 = 4;
	defUBO.sunShadow = 5;

	fbo->ReadBind();
	graphicsWrapper->SetDepth(0);
	quadShader->Use();

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	fbo->BindTexture(3);
	CubemapComponent *comp = engine.cubemapSystem.GetClosestCubemap(eyePos);
	if (comp != NULL) {
		Texture *cube = engine.cubemapSystem.GetClosestCubemap(eyePos)->cubemap;
		if (cube != NULL)
			cube->BindCubemap(4);
	}
	directionalLight->ReadBind();
	directionalLight->BindDepth(5);


	float t = (float)engine.GetTimeCurrent() / 4.0f;
	glm::vec3 pos = glm::vec3(0, sin(t), cos(t))*20.0f;
	glm::mat4 proj = glm::ortho<float>(-64, 64, -64, 64, -8, 64);
	glm::mat4 view = glm::lookAt(
		pos,
		glm::vec3(0, 0, 0),
		glm::cross(pos, glm::vec3(1, 0, 0))
	);

	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	defUBO.directionalShadowMatrix = biasMatrix * proj * view * glm::mat4(1.0f);

	quadShader->PassData(&defUBO);
	quadShader->SetUniformFloat();
	quadShader->SetVec3();
	quadShader->SetInteger();
	quadShader->SetInteger();
	quadShader->SetInteger();
	quadShader->SetInteger();
	quadShader->SetInteger();
	quadShader->SetInteger();
	quadShader->SetUniform4m();

	graphicsWrapper->Clear(CLEAR_COLOR);

	vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	vaoQuad->Unbind();
}

void RenderPathDeferred::PostPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	return;
	//fbo->TestBlit();

	engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
	skyShader->Use();
	skydefUBO.gWVP = projection * view * glm::translate(eyePos);
	skydefUBO.time = (float)engine.GetTimeCurrent();
	skyShader->PassData(&skydefUBO);
	skyShader->SetUniform4m();
	skyShader->SetUniformFloat();

	vaoSphere->Bind();
	graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
	vaoSphere->Unbind();
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

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile("../models/sphere12.obj",
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!pScene) {
		printf("%s", importer.GetErrorString());
		return;
	}

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(pScene->mMeshes[0]->mNumVertices);
	indices.reserve(pScene->mMeshes[0]->mNumFaces * 3);
	for (unsigned int i = 0; i < pScene->mMeshes[0]->mNumVertices; i++) {
		const aiVector3D* pPos = &(pScene->mMeshes[0]->mVertices[i]);
		vertices.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
	}

	for (unsigned int i = 0; i < pScene->mMeshes[0]->mNumFaces; i++) {
		const aiFace& Face = pScene->mMeshes[0]->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}

	numSkyIndices = indices.size();

	vaoSphere= pfnCreateVAO();
	vaoSphere->Initialize();
	vaoSphere->Bind();

	vboSphere = pfnCreateVBO();
	vboSphere->Initialize(1);
	vboSphere->AddVBO(&vertices[0], vertices.size() * sizeof(vertices[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vboSphere->Bind(0, 0, false, 0, 0);
	vboSphere->AddIBO(&indices[0], indices.size() * sizeof(unsigned int), DRAW_STATIC);

	vaoSphere->Unbind();

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

	directionalLight = pfnCreateFramebuffer();
	directionalLight->Initialize(0);
	directionalLight->AddDepthBuffer(1024, 1024);
	directionalLight->Generate();

	std::string vsPath = "../shaders/deferred.glvs";
	std::string fsPath = "../shaders/deferred.glfs";

	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	quadShader = pfnCreateShader();
	quadShader->Initialize(2);
	if (!quadShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!quadShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!quadShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	quadShader->SetNumUniforms(9);
	quadShader->CreateUniform("time");
	quadShader->CreateUniform("eyePos");
	quadShader->CreateUniform("texPos");
	quadShader->CreateUniform("texNormal");
	quadShader->CreateUniform("texAlbedo");
	quadShader->CreateUniform("texSpecular");
	quadShader->CreateUniform("texRefl");
	quadShader->CreateUniform("sunShadow");
	quadShader->CreateUniform("directionalShadowMatrix");
	
	vsContent.clear();
	fsContent.clear();
	vsPath.clear();
	fsPath.clear();

	vsPath = "../shaders/objects/sky.glvs";
	fsPath = "../shaders/objects/sky.glfs";

	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	skyShader = pfnCreateShader();
	skyShader->Initialize(2);
	if (!skyShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!skyShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!skyShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	skyShader->SetNumUniforms(2);
	skyShader->CreateUniform("gWVP");
	skyShader->CreateUniform("time");

	vsContent.clear();
	fsContent.clear();
	vsPath.clear();
	fsPath.clear();

	//terrain.Initialize();
	//terrain.LoadTerrain("../materials/height.png", 32, 32, 8, 8);

	envMap = LoadCubemap("../materials/skybox/Cliff", ".tga", COLOR_SRGB);
}

void RenderPathDeferred::Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, glm::vec2 res) {
	ShadowPass();
	GeometryPass(projection, view, eyePos);
	DeferredPass(eyePos, res);
	//PostPass(projection, view, eyePos);
}
