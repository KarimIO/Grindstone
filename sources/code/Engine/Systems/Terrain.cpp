#include "Terrain.h"
#include "../Core/Utilities.h"
#include "../Core/Engine.h"
#include "../Core/TextureManager.h"

void STerrain::Initialize() {
	std::string vsPath = "../shaders/objects/terrain.glvs";
	std::string fsPath = "../shaders/objects/terrain.glfs";
	std::string csPath = "../shaders/objects/terrain.glcs";
	std::string esPath = "../shaders/objects/terrain.gles";

	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	std::string csContent;
	if (!ReadFile(csPath, csContent))
		fprintf(stderr, "Failed to read tesselation control shader: %s.\n", csPath.c_str());

	std::string esContent;
	if (!ReadFile(esPath, esContent))
		fprintf(stderr, "Failed to read tesselation evaluation shader: %s.\n", esPath.c_str());

	terrainShader = pfnCreateShader();
	terrainShader->Initialize(4);
	if (!terrainShader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!terrainShader->AddShader(&csPath, &csContent, SHADER_TESS_CONTROL))
		fprintf(stderr, "Failed to add tesselation control shader %s.\n", csPath.c_str());
	if (!terrainShader->AddShader(&esPath, &esContent, SHADER_TESS_EVALUATION))
		fprintf(stderr, "Failed to add tesselation evaluation shader %s.\n", esPath.c_str());
	if (!terrainShader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!terrainShader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	vsContent.clear();
	fsContent.clear();
	csContent.clear();
	esContent.clear();
	vsPath.clear();
	fsPath.clear();
	csPath.clear();
	esPath.clear();

	terrainShader->SetNumUniforms(11);
	terrainShader->CreateUniform("gWorld");
	terrainShader->CreateUniform("gVP");
	terrainShader->CreateUniform("height");
	terrainShader->CreateUniform("time");
	terrainShader->CreateUniform("numPatches");
	terrainShader->CreateUniform("eyePos");
	terrainShader->CreateUniform("terrainMap");
	terrainShader->CreateUniform("grassAlbedo");
	terrainShader->CreateUniform("grassGeometry");
	terrainShader->CreateUniform("rockAlbedo");
	terrainShader->CreateUniform("rockGeometry");

	grassAlbedo = LoadTexture("../materials/terrain/grass/albedo.png", COLOR_SRGB);
	grassGeometry = LoadTexture("../materials/terrain/grass/geometry.png", COLOR_RGBA);

	rockAlbedo = LoadTexture("../materials/terrain/rock/albedo.png", COLOR_SRGB);
	rockGeometry = LoadTexture("../materials/terrain/rock/geometry.png", COLOR_RGBA);
}

void STerrain::LoadTerrain(std::string path, float width, float length, float height, unsigned int patches) {
	int numVerts = patches + 1;
	int numVertsArea = numVerts * numVerts;

	components.push_back(CTerrain());
	CTerrain *terrain = &components.back();
	terrain->texture = LoadTexture(path, COLOR_RGBA);
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;
	vertices.reserve(numVertsArea);
	texCoords.reserve(numVertsArea);
	indices.reserve(patches * patches * 6);
	terrain->numIndices = patches * patches * 6;
	terrain->numPatches = patches;
	float patchWidth	= width / (float)patches;
	float patchLength	= length / (float)patches;
	float widthStart	= -width / 2.0f;
	float lengthStart	= -length / 2.0f;
	terrain->height = height;

	for (size_t i = 0; i <= patches; i++) {
		for (size_t j = 0; j <= patches; j++) {
			vertices.push_back(glm::vec2(widthStart + patchWidth * j, lengthStart + patchLength * i));
			texCoords.push_back(glm::vec2((float)j/(float)patches, (float)i / (float)patches));
		}
	}

	for (size_t i = 0; i < patches; i++) {
		for (size_t j = 0; j < patches; j++) {
			indices.push_back((unsigned int)(i*numVerts + j));
			indices.push_back((unsigned int)((i + 1)*numVerts + j));
			indices.push_back((unsigned int)((i + 1)*numVerts + j + 1));
			indices.push_back((unsigned int)((i + 1)*numVerts + j + 1));
			indices.push_back((unsigned int)(i*numVerts + j + 1));
			indices.push_back((unsigned int)(i*numVerts + j));
		}
	}

	terrain->vao = pfnCreateVAO();
	terrain->vao->Initialize();
	terrain->vao->Bind();

	VertexBufferObject *vbo = pfnCreateVBO();
	vbo->Initialize(3);
	vbo->AddVBO(&vertices[0], vertices.size() * sizeof(vertices[0]), 2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(0, 0, false, 0, 0);
	vbo->AddVBO(&texCoords[0], texCoords.size() * sizeof(texCoords[0]), 2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(1, 1, false, 0, 0);
	/*vbo->AddVBO(&normals[0], normals.size() * sizeof(normals[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(2, 2, false, 0, 0);
	vbo->AddVBO(&tangents[0], tangents.size() * sizeof(tangents[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(3, 3, false, 0, 0);*/
	vbo->AddIBO(&indices[0], indices.size() * sizeof(unsigned int), DRAW_STATIC); // 3 and SIZE_FLOAT are arbitrary

	terrain->vao->Unbind();

	vertices.clear();
	texCoords.clear();
	indices.clear();
}

struct TerrainContainer {
	glm::mat4 gWorld;
	glm::mat4 gVP;
	float height;
	float time;
	int numPatches;
	glm::vec3 eyePos;
	int terrainMap;
	int grassAlbedo;
	int grassGeometry;
	int rockAlbedo;
	int rockGeometry;
} terrainContainer;

void STerrain::Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos) {
	//engine.graphicsWrapper->SetTesselation(3);
	terrainContainer.gWorld = glm::mat4(1);
	terrainContainer.gVP = projection * view;
	terrainContainer.eyePos = eyePos;
	terrainContainer.terrainMap = 0;
	terrainContainer.grassAlbedo = 1;
	terrainContainer.grassGeometry = 2;
	terrainContainer.rockAlbedo = 3;
	terrainContainer.rockGeometry = 4;
	terrainContainer.time = (float)engine.GetTimeCurrent();
	terrainShader->Use();
	grassAlbedo->Bind(1);
	grassGeometry->Bind(2);
	rockAlbedo->Bind(3);
	rockGeometry->Bind(4);

	for (size_t i = 0; i < components.size(); i++) {
		terrainContainer.height = components[i].height;
		terrainContainer.numPatches = components[i].numPatches;

		terrainShader->PassData(&terrainContainer);
		terrainShader->SetUniform4m();
		terrainShader->SetUniform4m();
		terrainShader->SetUniformFloat();
		terrainShader->SetUniformFloat();
		terrainShader->SetInteger();
		terrainShader->SetVec3();
		terrainShader->SetInteger();
		terrainShader->SetInteger();
		terrainShader->SetInteger();
		terrainShader->SetInteger();
		terrainShader->SetInteger();

		components[i].vao->Bind();
		components[i].texture->Bind(0);
		engine.graphicsWrapper->DrawBaseVertex(SHAPE_PATCHES, (void*)(0), 0, components[i].numIndices);
		components[i].vao->Unbind();
	}
	//engine.graphicsWrapper->SetTesselation(0);
}