#include "RenderPathDeferred.h"
#include "../Core/GraphicsDLLPointer.h"
#include "../Core/Utilities.h"
#include "../Core/Engine.h"
#include "gl3w.h"

#include "../Core/TextureManager.h"

#include <glm/gtx/transform.hpp>

struct IBLBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer3;
	int cubemap;
} iblUBO;

struct DirectionalLightBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int lightShadow;
	float lightSourceRadius;
	glm::mat4 shadowMatrix;
	glm::vec3 lightColor;
	float lightIntensity;
	glm::vec3 lightDirection;
} dirLightUBO;

struct PointLightBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int lightShadow;
	float lightAttenuationRadius;
	glm::mat4 shadowMatrix;
	glm::mat4 gWVP;
	glm::vec3 lightColor;
	float lightIntensity;
	glm::vec3 lightPosition;
} pointLightUBO;

struct SpotLightBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int lightShadow;
	float lightAttenuationRadius;
	float lightInnerAngle;
	float lightOuterAngle;
	glm::mat4 shadowMatrix;
	glm::mat4 gWVP;
	glm::vec3 lightColor;
	float lightIntensity;
	glm::vec3 lightPosition;
	glm::vec3 lightDirection;
} spotLightUBO;

struct SkyUniformBufferDef {
	glm::mat4 gWVP;
	float time;
} skydefUBO;

struct PostUniformBufferDef {
	int texLoc0;
	int texLoc1;
	int texLoc2;
} postUBO;

void RenderPathDeferred::GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	// Uses screen resolution due to framebuffer size
	fbo->WriteBind();
	graphicsWrapper->SetDepth(1);
	graphicsWrapper->SetCull(CULL_BACK);
	graphicsWrapper->Clear(CLEAR_ALL);
	graphicsWrapper->SetBlending(false);
	geometryCache->Draw(projection, view);
	//terrain.Draw(projection, view, eyePos);
	fbo->Unbind();
}

void RenderPathDeferred::DeferredPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) {
	dirLightUBO.eyePos = spotLightUBO.eyePos = pointLightUBO.eyePos = eyePos;
	dirLightUBO.gbuffer0 = spotLightUBO.gbuffer0 = pointLightUBO.gbuffer0 = 0;
	dirLightUBO.gbuffer1 = spotLightUBO.gbuffer1 = pointLightUBO.gbuffer1 = 1;
	dirLightUBO.gbuffer2 = spotLightUBO.gbuffer2 = pointLightUBO.gbuffer2 = 2;
	dirLightUBO.gbuffer3 = spotLightUBO.gbuffer3 = pointLightUBO.gbuffer3 = 3;

	fbo->ReadBind();
	graphicsWrapper->SetDepth(0);

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	fbo->BindTexture(3);
	fbo->Unbind();

	graphicsWrapper->Clear(CLEAR_COLOR);
	graphicsWrapper->SetBlending(true);

	pointLightShader->Use();
	glm::mat4 pv = projection * view;
	for (size_t i = 0; i < engine.lightSystem.pointLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.pointLights[i].entityID;
		EBase *entity = &engine.entities[entityID];
		CPointLight *light = &engine.lightSystem.pointLights[i];
		pointLightUBO.gWVP = pv * entity->GetModelMatrix();
		pointLightUBO.lightAttenuationRadius = light->lightRadius;
		pointLightUBO.lightColor = light->lightColor;
		pointLightUBO.lightIntensity = light->intensity;
		pointLightUBO.lightPosition = entity->GetPosition();
		pointLightUBO.lightShadow = 4;

		// Temporary disable pointlight shadows until we get them working
		if (false && light->castShadow) {
			light->fbo->ReadBind();
			light->fbo->BindDepthCube(4);
			light->fbo->Unbind();
		}

		pointLightShader->PassData(&pointLightUBO);
		pointLightShader->SetVec3();
		pointLightShader->SetInteger();
		pointLightShader->SetInteger();
		pointLightShader->SetInteger();
		pointLightShader->SetInteger();
		pointLightShader->SetInteger();
		pointLightShader->SetUniformFloat();
		pointLightShader->SetUniform4m();
		pointLightShader->SetUniform4m();
		pointLightShader->SetVec3();
		pointLightShader->SetUniformFloat();
		pointLightShader->SetVec3();

		vaoSphere->Bind();
		graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
		vaoSphere->Unbind();
	}

	spotLightShader->Use();
	for (size_t i = 0; i < engine.lightSystem.spotLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.spotLights[i].entityID;
		CSpotLight *light = &engine.lightSystem.spotLights[i];
		EBase *entity = &engine.entities[entityID];
		spotLightUBO.gWVP = pv * entity->GetModelMatrix();
		spotLightUBO.lightAttenuationRadius = light->lightRadius;
		spotLightUBO.lightInnerAngle = light->innerSpotAngle;
		spotLightUBO.lightOuterAngle = light->outerSpotAngle;
		spotLightUBO.lightColor = light->lightColor;
		spotLightUBO.lightIntensity = light->intensity;
		spotLightUBO.lightPosition = entity->GetPosition();
		spotLightUBO.lightDirection = entity->GetForward();
		spotLightUBO.lightShadow = 4;
		spotLightUBO.shadowMatrix = light->projection;

		if (light->castShadow) {
			light->fbo->ReadBind();
			light->fbo->BindDepth(4);
			light->fbo->Unbind();
		}

		spotLightShader->PassData(&spotLightUBO);
		spotLightShader->SetVec3();
		spotLightShader->SetInteger();
		spotLightShader->SetInteger();
		spotLightShader->SetInteger();
		spotLightShader->SetInteger();
		spotLightShader->SetInteger();
		spotLightShader->SetUniformFloat();
		spotLightShader->SetUniformFloat();
		spotLightShader->SetUniformFloat();
		spotLightShader->SetUniform4m();
		spotLightShader->SetUniform4m();
		spotLightShader->SetVec3();
		spotLightShader->SetUniformFloat();
		spotLightShader->SetVec3();
		spotLightShader->SetVec3();

		vaoSphere->Bind();
		graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
		vaoSphere->Unbind();
	}

	directionalLightShader->Use();
	for (size_t i = 0; i < engine.lightSystem.directionalLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.directionalLights[i].entityID;
		CDirectionalLight *light = &engine.lightSystem.directionalLights[i];
		EBase *entity = &engine.entities[entityID];
		dirLightUBO.lightColor = light->lightColor;
		dirLightUBO.lightIntensity = light->intensity;
		dirLightUBO.lightShadow = 4;
		dirLightUBO.shadowMatrix = light->projection;
		dirLightUBO.lightDirection = entity->GetForward();

		if (light->castShadow) {
			light->fbo->ReadBind();
			light->fbo->BindDepth(4);
			light->fbo->Unbind();
		}

		directionalLightShader->PassData(&dirLightUBO);
		directionalLightShader->SetVec3();
		directionalLightShader->SetInteger();
		directionalLightShader->SetInteger();
		directionalLightShader->SetInteger();
		directionalLightShader->SetInteger();
		directionalLightShader->SetInteger();
		directionalLightShader->SetUniformFloat();
		directionalLightShader->SetUniform4m();
		directionalLightShader->SetVec3();
		directionalLightShader->SetUniformFloat();
		directionalLightShader->SetVec3();

		vaoQuad->Bind();
		graphicsWrapper->DrawVertexArray(4);
		vaoQuad->Unbind();
	}

	if (usePost) {
		iblUBO.eyePos = eyePos;
		iblUBO.gbuffer0 = 0;
		iblUBO.gbuffer1 = 1;
		iblUBO.gbuffer3 = 3;
		iblUBO.cubemap = 4;

		CubemapComponent *comp = engine.cubemapSystem.GetClosestCubemap(eyePos);
		if (comp != NULL) {
			Texture *cube = engine.cubemapSystem.GetClosestCubemap(eyePos)->cubemap;
			if (cube != NULL)
				cube->BindCubemap(4);
		}

		iblShader->Use();

		iblShader->PassData(&iblUBO);
		iblShader->SetVec3();
		iblShader->SetInteger();
		iblShader->SetInteger();
		iblShader->SetInteger();
		iblShader->SetInteger();

		vaoQuad->Bind();
		graphicsWrapper->DrawVertexArray(4);
		vaoQuad->Unbind();
	}
	
	/*CubemapComponent *comp = engine.cubemapSystem.GetClosestCubemap(eyePos);
	if (comp != NULL) {
		Texture *cube = engine.cubemapSystem.GetClosestCubemap(eyePos)->cubemap;
		if (cube != NULL)
			cube->BindCubemap(4);
	}*/

	/*float t = (float)engine.GetTimeCurrent() / 4.0f;
	glm::vec3 pos = glm::vec3(0, sin(t), cos(t))*20.0f;
	glm::mat4 proj = glm::ortho<float>(-64, 64, -64, 64, -8, 64);
	glm::mat4 viewb = glm::lookAt(
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

	defUBO.directionalShadowMatrix = biasMatrix * proj * viewb * glm::mat4(1.0f);*/

	//postFBO->WriteBind();
}

void RenderPathDeferred::PostPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	return;
	engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
	/*//fbo->TestBlit();
	skyShader->Use();
	skydefUBO.gWVP = projection * view * glm::translate(eyePos);
	skydefUBO.time = (float)engine.GetTimeCurrent();
	skyShader->PassData(&skydefUBO);
	skyShader->SetUniform4m();
	skyShader->SetUniformFloat();

	vaoSphere->Bind();
	graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
	vaoSphere->Unbind();*/

	postUBO.texLoc0 = 0;
	postUBO.texLoc1 = 1;
	postUBO.texLoc2 = 2;

	fbo->ReadBind();
	graphicsWrapper->SetDepth(0);
	postShader->Use();

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	
	postShader->PassData(&postUBO);
	postShader->SetInteger();
	postShader->SetInteger();
	postShader->SetInteger();

	graphicsWrapper->Clear(CLEAR_COLOR);

	vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	vaoQuad->Unbind();
	fbo->Unbind();
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

	numSkyIndices = (unsigned int) indices.size();

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
	unsigned int resx = (unsigned int)res.x;
	unsigned int resy = (unsigned int)res.y;
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	// Depth Buffer Issue:
	fbo->AddDepthBuffer(resx, resy);
	fbo->Generate();

	std::string vsPath = "../shaders/overlay.glvs";
	std::string fsPath = "../shaders/deferred/directional.glfs";

	std::string vsContent;
	if (!ReadFileIncludable(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	std::string fsContent;
	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	directionalLightShader = pfnCreateShader();
	directionalLightShader->Initialize(2);
	if (!directionalLightShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!directionalLightShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!directionalLightShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	directionalLightShader->SetNumUniforms(11);
	directionalLightShader->CreateUniform("eyePos");
	directionalLightShader->CreateUniform("gbuffer0");
	directionalLightShader->CreateUniform("gbuffer1");
	directionalLightShader->CreateUniform("gbuffer2");
	directionalLightShader->CreateUniform("gbuffer3");
	directionalLightShader->CreateUniform("lightShadow");
	directionalLightShader->CreateUniform("lightSourceRadius");
	directionalLightShader->CreateUniform("shadowMatrix");
	directionalLightShader->CreateUniform("lightColor");
	directionalLightShader->CreateUniform("lightIntensity");
	directionalLightShader->CreateUniform("lightDirection");

	fsContent.clear();
	fsPath.clear();

	fsPath = "../shaders/deferred/ibl.glfs";

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	iblShader = pfnCreateShader();
	iblShader->Initialize(2);
	if (!iblShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!iblShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!iblShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	iblShader->SetNumUniforms(5);
	iblShader->CreateUniform("eyePos");
	iblShader->CreateUniform("gbuffer0");
	iblShader->CreateUniform("gbuffer1");
	iblShader->CreateUniform("gbuffer3");
	iblShader->CreateUniform("texRefl");

	fsContent.clear();
	fsPath.clear();
	vsContent.clear();
	vsPath.clear();

	vsPath = "../shaders/deferred/light.glvs";
	fsPath = "../shaders/deferred/point.glfs";

	if (!ReadFileIncludable(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	pointLightShader = pfnCreateShader();
	pointLightShader->Initialize(2);
	if (!pointLightShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!pointLightShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!pointLightShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	pointLightShader->SetNumUniforms(12);
	pointLightShader->CreateUniform("eyePos");
	pointLightShader->CreateUniform("gbuffer0");
	pointLightShader->CreateUniform("gbuffer1");
	pointLightShader->CreateUniform("gbuffer2");
	pointLightShader->CreateUniform("gbuffer3");
	pointLightShader->CreateUniform("lightShadow");
	pointLightShader->CreateUniform("lightAttenuationRadius");
	pointLightShader->CreateUniform("shadowMatrix");
	pointLightShader->CreateUniform("gWVP");
	pointLightShader->CreateUniform("lightColor");
	pointLightShader->CreateUniform("lightIntensity");
	pointLightShader->CreateUniform("lightPosition");

	fsContent.clear();
	fsPath.clear();

	fsPath = "../shaders/deferred/spot.glfs";
	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	spotLightShader = pfnCreateShader();
	spotLightShader->Initialize(2);
	if (!spotLightShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!spotLightShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!spotLightShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	spotLightShader->SetNumUniforms(15);
	spotLightShader->CreateUniform("eyePos");
	spotLightShader->CreateUniform("gbuffer0");
	spotLightShader->CreateUniform("gbuffer1");
	spotLightShader->CreateUniform("gbuffer2");
	spotLightShader->CreateUniform("gbuffer3");
	spotLightShader->CreateUniform("lightShadow");
	spotLightShader->CreateUniform("lightAttenuationRadius");
	spotLightShader->CreateUniform("lightInnerAngle");
	spotLightShader->CreateUniform("lightOuterAngle");
	spotLightShader->CreateUniform("shadowMatrix");
	spotLightShader->CreateUniform("gWVP");
	spotLightShader->CreateUniform("lightColor");
	spotLightShader->CreateUniform("lightIntensity");
	spotLightShader->CreateUniform("lightPosition");
	spotLightShader->CreateUniform("lightDirection");

	fsContent.clear();
	fsPath.clear();
	vsContent.clear();
	vsPath.clear();

	/*vsPath = "../shaders/objects/sky.glvs";
	fsPath = "../shaders/objects/sky.glfs";

	if (!ReadFileIncludable(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	if (!ReadFileIncludable(fsPath, fsContent))
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

	vsPath = "../shaders/overlay.glvs";
	fsPath = "../shaders/post.glfs";

	if (!ReadFileIncludable(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	postShader = pfnCreateShader();
	postShader->Initialize(2);
	if (!postShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!postShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!postShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	postShader->SetNumUniforms(3);
	postShader->CreateUniform("texPosition");
	postShader->CreateUniform("texNormals");
	postShader->CreateUniform("texDeferLit");

	vsContent.clear();
	fsContent.clear();
	vsPath.clear();
	fsPath.clear();*/
		
	postFBO = pfnCreateFramebuffer();
	postFBO->Initialize(1);
	postFBO->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, res.x, res.y);
	postFBO->Generate();


	//terrain.Initialize();
	//terrain.LoadTerrain("../materials/height.png", 32, 32, 8, 8);

	envMap = LoadCubemap("../materials/skybox/Cliff", ".tga", COLOR_SRGB);
}

void RenderPathDeferred::Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) {
	GeometryPass(projection, view, eyePos);
	DeferredPass(projection, view, eyePos, usePost);
	//PostPass(projection, view, eyePos);
}
