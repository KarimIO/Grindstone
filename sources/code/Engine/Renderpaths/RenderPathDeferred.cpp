#include "RenderPathDeferred.h"
#include "../Core/GraphicsDLLPointer.h"
#include "../Core/Utilities.h"
#include "../Core/Engine.h"

#include "../Systems/Terrain.h"
#include "../Core/TextureManager.h"

#include <glm/gtx/transform.hpp>

struct IBLBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int cubemap;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
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
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} dirLightUBO;

struct PointLightBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	float lightAttenuationRadius;
	glm::mat4 gWVP;
	glm::vec3 lightColor;
	float lightIntensity;
	glm::vec3 lightPosition;
	glm::vec2 resolution;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} pointLightUBO;

struct PointLightShadowBufferDef {
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
	glm::vec2 resolution;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} pointLightShadowUBO;

struct SpotLightBufferDef {
	glm::vec3 eyePos;
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	float lightAttenuationRadius;
	float lightInnerAngle;
	float lightOuterAngle;
	glm::mat4 gWVP;
	glm::vec3 lightColor;
	float lightIntensity;
	glm::vec3 lightPosition;
	glm::vec3 lightDirection;
	glm::vec2 resolution;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} spotLightUBO;

struct SpotLightShadowBufferDef {
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
	glm::vec2 resolution;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} spotLightShadowUBO;

struct SkyUniformBufferDef {
	glm::mat4 gWVP;
	float time;
} skydefUBO;

struct PostUniformBufferDef {
	int texLoc0;
	int texLoc1;
	int texLoc2;
} postUBO;

struct DebugUniformBufferDef {
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int texRefl;
	int debugMode;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
} debugUBO;

struct SSAOUniformBufferDef {
	int gbuffer0;
	int gbuffer1;
	int gbuffer2;
	int gbuffer3;
	int texNormals;
	glm::mat4 invProjMat;
	glm::mat4 invViewMat;
	glm::mat4 projection;
	glm::vec3 eyePos;
	float kernels[64*3];
} ssaoUBO;

struct SSAOBlurUniformBufferDef {
	int input;
} ssaoBlurUBO;

void RenderPathDeferred::GeometryPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	// Uses screen resolution due to framebuffer size
	fbo->WriteBind();

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	graphicsWrapper->SetDepthMask(true);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	graphicsWrapper->SetDepth(1);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	graphicsWrapper->SetCull(CULL_BACK);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	graphicsWrapper->Clear(CLEAR_ALL);
	graphicsWrapper->SetBlending(false);
	geometryCache->Draw(projection, view);
	terrainSystem->Draw(projection, view, eyePos);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	fbo->GenerateMipmap(0);
	fbo->GenerateMipmap(1);
	fbo->GenerateMipmap(2);
	fbo->GenerateMipmap(3);
	fbo->Unbind();

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	graphicsWrapper->SetDepthMask(false);
	graphicsWrapper->SetDepth(0);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";
}

void RenderPathDeferred::SSAOPrepass(glm::mat4 projection) {

	ssaoUBO.projection = projection;

	graphicsWrapper->Clear(CLEAR_DEPTH);
	graphicsWrapper->SetDepth(0);

	graphicsWrapper->SetColorMask(COLOR_MASK_ALPHA);
	graphicsWrapper->SetBlending(false);

	ssaoNoiseTex->Bind(4);

	ssaoShader->Use();

	ssaoShader->PassData(&ssaoUBO);
	ssaoShader->SetInteger();
	ssaoShader->SetInteger();
	ssaoShader->SetInteger();
	ssaoShader->SetInteger();
	ssaoShader->SetInteger();
	ssaoShader->SetUniform4m();
	ssaoShader->SetUniform4m();
	ssaoShader->SetUniform4m();
	ssaoShader->SetVec3();
	ssaoShader->SetFloatArray(64);

	fbo->WriteBind();
	engine.engine.vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	engine.vaoQuad->Unbind();
	fbo->Unbind();
	graphicsWrapper->SetColorMask(COLOR_MASK_ALL);

	/*graphicsWrapper->Clear(CLEAR_ALL);
	graphicsWrapper->SetBlending(false);
	ssaoBlurShader->Use();

	ssaoBlurShader->PassData(&ssaoBlurUBO);
	ssaoBlurShader->SetInteger();

	engine.vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	engine.vaoQuad->Unbind();*/
	
}

void RenderPathDeferred::DeferredPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) {
	iblUBO.invProjMat = debugUBO.invProjMat = dirLightUBO.invProjMat = spotLightShadowUBO.invProjMat = spotLightUBO.invProjMat = pointLightShadowUBO.invProjMat = pointLightUBO.invProjMat = glm::inverse(projection);
	ssaoUBO.invViewMat = iblUBO.invViewMat = debugUBO.invViewMat = dirLightUBO.invViewMat = spotLightShadowUBO.invViewMat = spotLightUBO.invViewMat = pointLightShadowUBO.invViewMat = pointLightUBO.invViewMat = glm::inverse(view);

	postFBO->WriteBind();

	graphicsWrapper->Clear(CLEAR_COLOR);

	if (engine.debugMode != DEBUG_NONE) {
		debugUBO.debugMode = engine.debugMode;
		debugUBO.texRefl = 4;

		if (engine.lightSystem.directionalLights.size() > 0) {
			CDirectionalLight *light = &engine.lightSystem.directionalLights[0];
			if (light->castShadow) {
				light->fbo->ReadBind();
				light->fbo->BindDepth(4);
				light->fbo->UnbindRead();
			}
		}

		debugShader->Use();
		debugShader->PassData(&debugUBO);
		debugShader->SetInteger();
		debugShader->SetInteger();
		debugShader->SetInteger();
		debugShader->SetInteger();
		debugShader->SetInteger();
		debugShader->SetInteger();
		debugShader->SetUniform4m();
		debugShader->SetUniform4m();

		engine.vaoQuad->Bind();
		graphicsWrapper->DrawVertexArray(4);
		engine.vaoQuad->Unbind();
		return;
	}

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	ssaoUBO.eyePos = dirLightUBO.eyePos = spotLightUBO.eyePos = spotLightShadowUBO.eyePos = pointLightShadowUBO.eyePos = pointLightUBO.eyePos = eyePos;
	spotLightUBO.resolution = spotLightShadowUBO.resolution = pointLightShadowUBO.resolution = pointLightUBO.resolution = glm::vec2(engine.settings.resolutionX, engine.settings.resolutionY);

	graphicsWrapper->SetBlending(true);
	graphicsWrapper->SetDepthMask(false);

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	glm::mat4 pv = projection * view;
	for (size_t i = 0; i < engine.lightSystem.pointLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.pointLights[i].entityID;
		EBase *entity = &engine.entities[entityID];
		CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
		CPointLight *light = &engine.lightSystem.pointLights[i];
		

		if (light->castShadow) {
			pointLightShadowShader->Use();
			pointLightShadowUBO.gWVP = pv * transform->GetModelMatrix();
			pointLightShadowUBO.lightAttenuationRadius = light->lightRadius;
			pointLightShadowUBO.lightColor = light->lightColor;
			pointLightShadowUBO.lightIntensity = light->intensity;
			pointLightShadowUBO.lightPosition = transform->GetPosition();
			pointLightShadowUBO.lightShadow = 4; 
			
			light->fbo->ReadBind();
			light->fbo->BindDepthCube(4);
			light->fbo->UnbindRead();

			pointLightShadowShader->PassData(&pointLightShadowUBO);
			pointLightShadowShader->SetVec3();
			pointLightShadowShader->SetInteger();
			pointLightShadowShader->SetInteger();
			pointLightShadowShader->SetInteger();
			pointLightShadowShader->SetInteger();
			pointLightShadowShader->SetInteger();
			pointLightShadowShader->SetUniformFloat();
			pointLightShadowShader->SetUniform4m();
			pointLightShadowShader->SetUniform4m();
			pointLightShadowShader->SetVec3();
			pointLightShadowShader->SetUniformFloat();
			pointLightShadowShader->SetVec3();
			pointLightShadowShader->SetVec2();
			pointLightShadowShader->SetUniform4m();
			pointLightShadowShader->SetUniform4m();
		}
		else {
			pointLightShader->Use();
			pointLightUBO.gWVP = pv * transform->GetModelMatrix();
			pointLightUBO.lightAttenuationRadius = light->lightRadius;
			pointLightUBO.lightColor = light->lightColor;
			pointLightUBO.lightIntensity = light->intensity;
			pointLightUBO.lightPosition = transform->GetPosition();

			pointLightShader->PassData(&pointLightUBO);
			pointLightShader->SetVec3();
			pointLightShader->SetInteger();
			pointLightShader->SetInteger();
			pointLightShader->SetInteger();
			pointLightShader->SetInteger();
			pointLightShader->SetUniformFloat();
			pointLightShader->SetUniform4m();
			pointLightShader->SetVec3();
			pointLightShader->SetUniformFloat();
			pointLightShader->SetVec3();
			pointLightShader->SetVec2();
			pointLightShader->SetUniform4m();
			pointLightShader->SetUniform4m();
		}

		vaoSphere->Bind();
		graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
		vaoSphere->Unbind();
	}

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	for (size_t i = 0; i < engine.lightSystem.spotLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.spotLights[i].entityID;
		CSpotLight *light = &engine.lightSystem.spotLights[i];
		EBase *entity = &engine.entities[entityID];
		CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];

		if (light->castShadow) {
			spotLightShadowShader->Use();

			spotLightShadowUBO.gWVP = pv * transform->GetModelMatrix();
			spotLightShadowUBO.lightAttenuationRadius = light->lightRadius;
			spotLightShadowUBO.lightInnerAngle = light->innerSpotAngle;
			spotLightShadowUBO.lightOuterAngle = light->outerSpotAngle;
			spotLightShadowUBO.lightColor = light->lightColor;
			spotLightShadowUBO.lightIntensity = light->intensity;
			spotLightShadowUBO.lightPosition = transform->GetPosition();
			spotLightShadowUBO.lightDirection = transform->GetForward();
			spotLightShadowUBO.lightShadow = 4;
			spotLightShadowUBO.shadowMatrix = light->projection;

			light->fbo->ReadBind();
			light->fbo->BindDepth(4);
			light->fbo->UnbindRead();

			spotLightShadowShader->PassData(&spotLightShadowUBO);
			spotLightShadowShader->SetVec3();
			spotLightShadowShader->SetInteger();
			spotLightShadowShader->SetInteger();
			spotLightShadowShader->SetInteger();
			spotLightShadowShader->SetInteger();
			spotLightShadowShader->SetInteger();
			spotLightShadowShader->SetUniformFloat();
			spotLightShadowShader->SetUniformFloat();
			spotLightShadowShader->SetUniformFloat();
			spotLightShadowShader->SetUniform4m();
			spotLightShadowShader->SetUniform4m();
			spotLightShadowShader->SetVec3();
			spotLightShadowShader->SetUniformFloat();
			spotLightShadowShader->SetVec3();
			spotLightShadowShader->SetVec3();
			spotLightShadowShader->SetVec2();
			spotLightShadowShader->SetUniform4m();
			spotLightShadowShader->SetUniform4m();
		}
		else {
			spotLightShader->Use();

			spotLightUBO.gWVP = pv * transform->GetModelMatrix();
			spotLightUBO.lightAttenuationRadius = light->lightRadius;
			spotLightUBO.lightInnerAngle = light->innerSpotAngle;
			spotLightUBO.lightOuterAngle = light->outerSpotAngle;
			spotLightUBO.lightColor = light->lightColor;
			spotLightUBO.lightIntensity = light->intensity;
			spotLightUBO.lightPosition = transform->GetPosition();
			spotLightUBO.lightDirection = transform->GetForward();

			spotLightShader->PassData(&spotLightUBO);
			spotLightShader->SetVec3();
			spotLightShader->SetInteger();
			spotLightShader->SetInteger();
			spotLightShader->SetInteger();
			spotLightShader->SetInteger();
			spotLightShader->SetUniformFloat();
			spotLightShader->SetUniformFloat();
			spotLightShader->SetUniformFloat();
			spotLightShader->SetUniform4m();
			spotLightShader->SetVec3();
			spotLightShader->SetUniformFloat();
			spotLightShader->SetVec3();
			spotLightShader->SetVec3();
			spotLightShader->SetVec2();
			spotLightShader->SetUniform4m();
			spotLightShader->SetUniform4m();
		}


		vaoSphere->Bind();
		graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * 0), 0, numSkyIndices);
		vaoSphere->Unbind();
	}

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	directionalLightShader->Use();
	for (size_t i = 0; i < engine.lightSystem.directionalLights.size(); i++) {
		unsigned int entityID = engine.lightSystem.directionalLights[i].entityID;
		CDirectionalLight *light = &engine.lightSystem.directionalLights[i];
		EBase *entity = &engine.entities[entityID];
		CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
		dirLightUBO.lightColor = light->lightColor;
		dirLightUBO.lightIntensity = light->intensity;
		dirLightUBO.lightShadow = 4;
		dirLightUBO.shadowMatrix = light->projection;
		dirLightUBO.lightDirection = transform->GetForward();

		if (light->castShadow) {
			light->fbo->ReadBind();
			light->fbo->BindDepth(4);
			light->fbo->UnbindRead();
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
		directionalLightShader->SetUniform4m();
		directionalLightShader->SetUniform4m();

		engine.vaoQuad->Bind();
		graphicsWrapper->DrawVertexArray(4);
		engine.vaoQuad->Unbind();
	}

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	if (usePost) {
		//graphicsWrapper->SetBlending(false);

		iblUBO.eyePos = eyePos;
		iblUBO.gbuffer0 = 0;
		iblUBO.gbuffer1 = 1;
		iblUBO.gbuffer2 = 2;
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
		iblShader->SetInteger();
		iblShader->SetUniform4m();
		iblShader->SetUniform4m();

		engine.vaoQuad->Bind();
		graphicsWrapper->DrawVertexArray(4);
		engine.vaoQuad->Unbind();
	}

	return; // Hide Sky for now

	graphicsWrapper->SetBlending(false);
	graphicsWrapper->SetDepthMask(true);
	graphicsWrapper->SetDepth(2);
	
	fbo->ReadBind();
	fbo->TestBlit(0, 0, engine.settings.resolutionX, engine.settings.resolutionY, engine.settings.resolutionX, engine.settings.resolutionY, true);
	fbo->UnbindRead();

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";

	skyShader->Use();
	skydefUBO.gWVP = projection * glm::mat4(glm::mat3(view));
	skydefUBO.time = (float)engine.GetTimeCurrent();
	skyShader->PassData(&skydefUBO);
	skyShader->SetUniform4m();
	skyShader->SetUniformFloat();

	vaoSphere->Bind();
	graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, 0, 0, numSkyIndices);
	vaoSphere->Unbind();

	graphicsWrapper->SetDepth(1);

	postFBO->Unbind();

	if (graphicsWrapper->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";
}

void RenderPathDeferred::PostPass(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	return;
	postUBO.texLoc0 = 0;
	postUBO.texLoc1 = 1;
	postUBO.texLoc2 = 2;

	fbo->ReadBind();
	postShader->Use();

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	
	postShader->PassData(&postUBO);
	postShader->SetInteger();
	postShader->SetInteger();
	postShader->SetInteger();

	graphicsWrapper->Clear(CLEAR_COLOR);

	engine.vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	engine.vaoQuad->Unbind();
	fbo->Unbind();
}

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * gw, SModel * gc, STerrain *ts) {
	graphicsWrapper = gw;
	geometryCache = gc;
	terrainSystem = ts;

	BuildQuad();
	BuildSphere();
	SetupDeferredFBO();

	std::string vsPath, vsContent;
	CompileDirectionalShader(vsPath, vsContent);
	CompileIBLShader(vsPath, vsContent);
	CompileDebugShader(vsPath, vsContent);
	CompileSSAO(vsPath, vsContent);
	CompilePostShader(vsPath, vsContent);

	vsPath.clear();
	vsContent.clear();

	CompilePointShader(vsPath, vsContent);
	CompileSpotShader(vsPath, vsContent);

	CompilePointShadowShader(vsPath, vsContent);
	CompileSpotShadowShader(vsPath, vsContent);

	CompileSkyShader();


	BuildPostFBO();

	dirLightUBO.gbuffer0 = spotLightShadowUBO.gbuffer0 = spotLightUBO.gbuffer0 = pointLightShadowUBO.gbuffer0 = pointLightUBO.gbuffer0 = 0;
	dirLightUBO.gbuffer1 = spotLightShadowUBO.gbuffer1 = spotLightUBO.gbuffer1 = pointLightShadowUBO.gbuffer1 = pointLightUBO.gbuffer1 = 1;
	dirLightUBO.gbuffer2 = spotLightShadowUBO.gbuffer2 = spotLightUBO.gbuffer2 = pointLightShadowUBO.gbuffer2 = pointLightUBO.gbuffer2 = 2;
	dirLightUBO.gbuffer3 = spotLightShadowUBO.gbuffer3 = spotLightUBO.gbuffer3 = pointLightShadowUBO.gbuffer3 = pointLightUBO.gbuffer3 = 3;

	cube = engine.textureManager.LoadCubemap("../materials/skybox/Cliff", ".tga", COLOR_RGB);
}

inline void RenderPathDeferred::BuildQuad() {
	float tempVerts[8] = {
		-1,-1,
		1,-1,
		-1, 1,
		1, 1,
	};

	engine.vaoQuad = pfnCreateVAO();
	engine.vaoQuad->Initialize();
	engine.vaoQuad->Bind();

	engine.vboQuad = pfnCreateVBO();
	engine.vboQuad->Initialize(1);
	engine.vboQuad->AddVBO(tempVerts, sizeof(tempVerts), 2, SIZE_FLOAT, DRAW_STATIC);
	engine.vboQuad->Bind(0);
	engine.vaoQuad->Unbind();
}

inline void RenderPathDeferred::BuildSphere() {
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile("../models/sphere.obj",
		aiProcess_FlipWindingOrder |
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

	numSkyIndices = (unsigned int)indices.size();
	vaoSphere = pfnCreateVAO();
	vaoSphere->Initialize();
	vaoSphere->Bind();

	vboSphere = pfnCreateVBO();
	vboSphere->Initialize(2);
	vboSphere->AddVBO(&vertices[0], vertices.size() * sizeof(vertices[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vboSphere->Bind(0, 0, false, 0, 0);
	vboSphere->AddIBO(&indices[0], indices.size() * sizeof(unsigned int), DRAW_STATIC);

	vaoSphere->Unbind();

	vertices.clear();
	indices.clear();

	importer.FreeScene();
}

inline void RenderPathDeferred::SetupDeferredFBO() {
	glm::vec2 res = glm::vec2(engine.settings.resolutionX, engine.settings.resolutionY);
	fbo = pfnCreateFramebuffer();
	fbo->Initialize(4);
	unsigned int resx = (unsigned int)res.x;
	unsigned int resy = (unsigned int)res.y;
#ifdef _WIN32 // Remove this ASAP
	const int GL_RGBA16F = 0x881A;
	const int GL_RGBA = 0x1908;
	const int GL_FLOAT = 0x1406;
	const int GL_RGBA32F = 0x8814;
#endif
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	fbo->AddBuffer(GL_RGBA16F, GL_RGBA, GL_FLOAT, resx, resy);
	// Depth Buffer Issue:
	fbo->AddDepthBuffer(resx, resy);
	fbo->Generate();
}

inline void RenderPathDeferred::CompileDirectionalShader(std::string &vsPath, std::string &vsContent) {
	vsPath = "../shaders/overlay.glvs";
	std::string fsPath = "../shaders/deferred/directional.glfs";

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

	directionalLightShader->SetNumUniforms(13);
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
	directionalLightShader->CreateUniform("invProjMat");
	directionalLightShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompileIBLShader(std::string vsPath, std::string vsContent) {
	std::string fsContent, fsPath = "../shaders/deferred/ibl.glfs";

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	iblShader = pfnCreateShader();
	iblShader->Initialize(2);
	if (!iblShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!iblShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!iblShader->Compile())
		fprintf(stderr, "Failed to compile IBL Program\n");

	iblShader->SetNumUniforms(8);
	iblShader->CreateUniform("eyePos");
	iblShader->CreateUniform("gbuffer0");
	iblShader->CreateUniform("gbuffer1");
	iblShader->CreateUniform("gbuffer2");
	iblShader->CreateUniform("gbuffer3");
	iblShader->CreateUniform("texRefl");
	iblShader->CreateUniform("invProjMat");
	iblShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompilePointShader(std::string &vsPath, std::string &vsContent) {
	vsPath = "../shaders/deferred/light.glvs";
	std::string fsContent, fsPath = "../shaders/deferred/point.glfs";

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

	pointLightShader->SetNumUniforms(13);
	pointLightShader->CreateUniform("eyePos");
	pointLightShader->CreateUniform("gbuffer0");
	pointLightShader->CreateUniform("gbuffer1");
	pointLightShader->CreateUniform("gbuffer2");
	pointLightShader->CreateUniform("gbuffer3");
	pointLightShader->CreateUniform("lightAttenuationRadius");
	pointLightShader->CreateUniform("gWVP");
	pointLightShader->CreateUniform("lightColor");
	pointLightShader->CreateUniform("lightIntensity");
	pointLightShader->CreateUniform("lightPosition");
	pointLightShader->CreateUniform("resolution");
	pointLightShader->CreateUniform("invProjMat");
	pointLightShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompilePointShadowShader(std::string vsPath, std::string vsContent) {
	std::string fsContent, fsPath = "../shaders/deferred/pointShadow.glfs";

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	pointLightShadowShader = pfnCreateShader();
	pointLightShadowShader->Initialize(2);
	if (!pointLightShadowShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!pointLightShadowShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!pointLightShadowShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	pointLightShadowShader->SetNumUniforms(15);
	pointLightShadowShader->CreateUniform("eyePos");
	pointLightShadowShader->CreateUniform("gbuffer0");
	pointLightShadowShader->CreateUniform("gbuffer1");
	pointLightShadowShader->CreateUniform("gbuffer2");
	pointLightShadowShader->CreateUniform("gbuffer3");
	pointLightShadowShader->CreateUniform("lightShadow");
	pointLightShadowShader->CreateUniform("lightAttenuationRadius");
	pointLightShadowShader->CreateUniform("shadowMatrix");
	pointLightShadowShader->CreateUniform("gWVP");
	pointLightShadowShader->CreateUniform("lightColor");
	pointLightShadowShader->CreateUniform("lightIntensity");
	pointLightShadowShader->CreateUniform("lightPosition");
	pointLightShadowShader->CreateUniform("resolution");
	pointLightShadowShader->CreateUniform("invProjMat");
	pointLightShadowShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompileSpotShader(std::string vsPath, std::string vsContent) {
	std::string fsContent, fsPath = "../shaders/deferred/spot.glfs";
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

	spotLightShader->SetNumUniforms(16);
	spotLightShader->CreateUniform("eyePos");
	spotLightShader->CreateUniform("gbuffer0");
	spotLightShader->CreateUniform("gbuffer1");
	spotLightShader->CreateUniform("gbuffer2");
	spotLightShader->CreateUniform("gbuffer3");
	spotLightShader->CreateUniform("lightAttenuationRadius");
	spotLightShader->CreateUniform("lightInnerAngle");
	spotLightShader->CreateUniform("lightOuterAngle");
	spotLightShader->CreateUniform("gWVP");
	spotLightShader->CreateUniform("lightColor");
	spotLightShader->CreateUniform("lightIntensity");
	spotLightShader->CreateUniform("lightPosition");
	spotLightShader->CreateUniform("lightDirection");
	spotLightShader->CreateUniform("resolution");
	spotLightShader->CreateUniform("invProjMat");
	spotLightShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompileSpotShadowShader(std::string vsPath, std::string vsContent) {
	std::string fsContent, fsPath = "../shaders/deferred/spotShadow.glfs";
	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	spotLightShadowShader = pfnCreateShader();
	spotLightShadowShader->Initialize(2);
	if (!spotLightShadowShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!spotLightShadowShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!spotLightShadowShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	spotLightShadowShader->SetNumUniforms(18);
	spotLightShadowShader->CreateUniform("eyePos");
	spotLightShadowShader->CreateUniform("gbuffer0");
	spotLightShadowShader->CreateUniform("gbuffer1");
	spotLightShadowShader->CreateUniform("gbuffer2");
	spotLightShadowShader->CreateUniform("gbuffer3");
	spotLightShadowShader->CreateUniform("lightShadow");
	spotLightShadowShader->CreateUniform("lightAttenuationRadius");
	spotLightShadowShader->CreateUniform("lightInnerAngle");
	spotLightShadowShader->CreateUniform("lightOuterAngle");
	spotLightShadowShader->CreateUniform("shadowMatrix");
	spotLightShadowShader->CreateUniform("gWVP");
	spotLightShadowShader->CreateUniform("lightColor");
	spotLightShadowShader->CreateUniform("lightIntensity");
	spotLightShadowShader->CreateUniform("lightPosition");
	spotLightShadowShader->CreateUniform("lightDirection");
	spotLightShadowShader->CreateUniform("resolution");
	spotLightShadowShader->CreateUniform("invProjMat");
	spotLightShadowShader->CreateUniform("invViewMat");

	fsContent.clear();
	fsPath.clear();
	vsContent.clear();
	vsPath.clear();
}

inline void RenderPathDeferred::CompileSkyShader() {
	std::string vsPath = "../shaders/objects/sky.glvs";
	std::string fsPath = "../shaders/objects/sky.glfs";

	std::string vsContent, fsContent;
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
}

inline void RenderPathDeferred::CompileDebugShader(std::string vsPath, std::string vsContent) {
	std::string fsPath = "../shaders/debug/debug.glfs";
	std::string fsContent;
	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	debugShader = pfnCreateShader();
	debugShader->Initialize(2);
	if (!debugShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!debugShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!debugShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	debugShader->SetNumUniforms(8);
	debugShader->CreateUniform("gbuffer0");
	debugShader->CreateUniform("gbuffer1");
	debugShader->CreateUniform("gbuffer2");
	debugShader->CreateUniform("gbuffer3");
	debugShader->CreateUniform("texRefl");
	debugShader->CreateUniform("debugMode");
	debugShader->CreateUniform("invProjMat");
	debugShader->CreateUniform("invViewMat");

	debugUBO.gbuffer0 = 0;
	debugUBO.gbuffer1 = 1;
	debugUBO.gbuffer2 = 2;
	debugUBO.gbuffer3 = 3;

	vsContent.clear();
	fsContent.clear();
	vsPath.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::CompilePostShader(std::string vsPath, std::string vsContent) {
	std::string fsPath = "../shaders/post.glfs";
	std::string fsContent;
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
	postShader->CreateUniform("gbuffer0");
	postShader->CreateUniform("gbuffer1");
	postShader->CreateUniform("gbuffer2");

	vsContent.clear();
	fsContent.clear();
	vsPath.clear();
	fsPath.clear();
}

inline void RenderPathDeferred::BuildPostFBO() {
	glm::vec2 res = glm::vec2(engine.settings.resolutionX, engine.settings.resolutionY);
	unsigned int resx = (unsigned int)res.x;
	unsigned int resy = (unsigned int)res.y;
#ifdef _WIN32 // Remove this ASAP
	const int GL_RGBA = 0x1908;
	const int GL_FLOAT = 0x1406;
	const int GL_RGBA32F = 0x8814;
#endif

	postFBO = pfnCreateFramebuffer();
	postFBO->Initialize(1);
	postFBO->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned int)res.x, (unsigned int)res.y);
	postFBO->Generate();
}

inline void RenderPathDeferred::CompileSSAO(std::string vsPath, std::string vsContent) {

	for (int i = 0; i < 64; i++) {
		glm::vec3 sample;
		float angleX = 6.28318f * (float)(rand()) / (float)(RAND_MAX);
		sample.x = glm::cos(angleX);
		sample.y = glm::sin(angleX);
		float angleY = 3.14159f * (float)(rand()) / (float)(RAND_MAX);
		sample.z = glm::sin(angleY);
		float distance = (float)(rand()) / (float)(RAND_MAX);
		sample *= glm::clamp(distance*distance, 0.1f, 1.0f);
		ssaoUBO.kernels[i * 3  ] = sample.x;
		ssaoUBO.kernels[i * 3+1] = sample.y;
		ssaoUBO.kernels[i * 3+2] = sample.z;
	}

	std::vector<glm::vec2> ssaoNoise;
	ssaoNoise.reserve(4);
	for (int i = 0; i < 16; i++) {
		ssaoNoise.push_back(glm::vec2(
			2.0f * (float)(rand()) / (float)(RAND_MAX) - 1.0f,
			2.0f * (float)(rand()) / (float)(RAND_MAX) - 1.0f));
	}

	ssaoNoiseTex = engine.textureManager.LoadTexture("../materials/ssaoNoise.png", COLOR_RGBA);

	std::string fsContent, fsPath = "../shaders/ssao.glfs";

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	ssaoShader = pfnCreateShader();
	ssaoShader->Initialize(2);
	if (!ssaoShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!ssaoShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());

	ssaoShader->BindOutputLocation(2, "output");

	if (!ssaoShader->Compile())
		fprintf(stderr, "Failed to compile SSAO Program\n");

	ssaoShader->SetNumUniforms(10);
	ssaoShader->CreateUniform("gbuffer0");
	ssaoShader->CreateUniform("gbuffer1");
	ssaoShader->CreateUniform("gbuffer2");
	ssaoShader->CreateUniform("gbuffer3");
	ssaoShader->CreateUniform("noiseTex");
	ssaoShader->CreateUniform("invProjMat");
	ssaoShader->CreateUniform("invViewMat");
	ssaoShader->CreateUniform("projection");
	ssaoShader->CreateUniform("eyePos");
	ssaoShader->CreateUniform("kernels");

	ssaoUBO.gbuffer0 = 0;
	ssaoUBO.gbuffer1 = 1;
	ssaoUBO.gbuffer2 = 2;
	ssaoUBO.gbuffer3 = 3;
	ssaoUBO.texNormals = 4;

	fsContent.clear();
	fsPath.clear();

	fsPath = "../shaders/ssaoBlur.glfs";

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	ssaoBlurShader = pfnCreateShader();
	ssaoBlurShader->Initialize(2);
	if (!ssaoBlurShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!ssaoBlurShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());

	ssaoBlurShader->BindOutputLocation(2, "output");

	if (!ssaoBlurShader->Compile())
		fprintf(stderr, "Failed to compile SSAO Program\n");

	ssaoBlurShader->SetNumUniforms(1);
	ssaoBlurShader->CreateUniform("input");

	ssaoBlurUBO.input = 2;

	fsContent.clear();
	fsPath.clear();
}

void RenderPathDeferred::Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) {
	GeometryPass(projection, view, eyePos);

	fbo->ReadBind();
	graphicsWrapper->SetDepth(0);
	fbo->BindDepth(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);
	fbo->BindTexture(3);
	fbo->Unbind();

	//SSAOPrepass(projection);
	DeferredPass(projection, view, eyePos, usePost);
	PostPass(projection, view, eyePos);
}

Framebuffer *RenderPathDeferred::GetFramebuffer() {
	return postFBO;
}
