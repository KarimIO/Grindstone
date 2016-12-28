#include "Engine.h"

#include "SGeometry.h"
#include "GraphicsDLLPointer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::string CModel::getName() {
	return name;
}

CModel * SModel::PrepareModel3D(const char * szPath) {
	for (size_t i = 0; i < models.size(); i++)
		if (models[i].getName() == szPath)
			return &models[i];

	models.push_back(CModel());
	CModel *model = &models.back();
	model->name = szPath;

	return model;
}

CModel * SModel::LoadModel3D(const char * szPath) {
	for (size_t i = 0; i < models.size(); i++)
		if (models[i].getName() == szPath)
			return &models[i];

	models.push_back(CModel());
	CModel *model = &models.back();
	model->name = szPath;

	LoadModel3DFile(szPath, model);

	return model;
}

CModel * SModel::PrepareModel3D(const char * szPath, CRender *cRef) {
	for (size_t i = 0; i < models.size(); i++)
		if (models[i].getName() == szPath)
			return &models[i];

	models.push_back(CModel());
	CModel *model = &models.back();
	model->name = szPath;
	model->references.push_back(cRef);

	return model;
}

CModel * SModel::LoadModel3D(const char * szPath, CRender *cRef) {
	for (size_t i = 0; i < models.size(); i++)
		if (models[i].getName() == szPath)
			return &models[i];

	models.push_back(CModel());
	CModel *model = &models.back();
	model->name = szPath;
	model->references.push_back(cRef);

	LoadModel3DFile(szPath, model);

	return model;
}

void InitMesh(const aiMesh *paiMesh,
	std::vector<glm::vec3>& vertices,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec2>& uvs,
	std::vector<unsigned int>& indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
		const aiVector3D* pTangent = paiMesh->HasTangentsAndBitangents() ? &(paiMesh->mTangents[i]) : &Zero3D;
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		vertices.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		uvs.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
		normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		tangents.push_back(glm::vec3(-pTangent->x, -pTangent->y, -pTangent->z));
	}

	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}
}


void SwitchSlashes(std::string &path) {
	size_t index = 0;
	while (true) {
		/* Locate the substring to replace. */
		index = path.find("\\", index);
		if (index == std::string::npos) break;

		/* Make the replacement. */
		path.replace(index, 1, "/");

		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index += 1;
	}
}

Texture *LoadTexture(std::string path) {
	Texture *t = pfnCreateTexture();
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	t->CreateTexture(pixels, COLOR_RGBA, texWidth, texHeight);

	if (!pixels)
		printf("Texture failed to load!: %s \n", path.c_str());

	return t;
}

void SModel::InitMaterials(const aiScene* scene, std::string Dir, CModel *model)
{
	std::string finalDir = Dir;
	finalDir = finalDir.substr(0, finalDir.find_last_of('/'));

	aiMaterial *pMaterial;
	aiString Path;
	for (size_t i = 0; i < scene->mNumMaterials; i++) {
		Material *newMat = new Material();
		pMaterial = scene->mMaterials[i];
		
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = finalDir + "/" + Path.data;
				SwitchSlashes(FullPath);
				newMat->tex = LoadTexture(FullPath);
			}
		}
		model->materials[i] = newMat;
	}
}


void SModel::LoadModel3DFile(const char *szPath, CModel *model) {
	// Create an instance of the Importer class
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(szPath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!pScene) {
		printf("%s", importer.GetErrorString());
		return;
	}

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	model->meshes.resize(pScene->mNumMeshes);
	model->materials.resize(pScene->mNumMaterials);
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
		model->meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		model->meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		model->meshes[i].BaseVertex = NumVertices;
		model->meshes[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += model->meshes[i].NumIndices;
	}


	//std::cout << "Mesh Materials: " << path << "\n";
	InitMaterials(pScene, szPath, model);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	vertices.reserve(NumVertices);
	normals.reserve(NumVertices);
	tangents.reserve(NumVertices);
	uvs.reserve(NumVertices);
	indices.reserve(NumIndices);

	std::cout << "Loading Mesh: " << szPath << "\n";

	for (size_t i = 0; i < pScene->mNumMeshes; i++) {
		InitMesh(pScene->mMeshes[i], vertices, normals, tangents, uvs, indices);
	}

	model->vao = pfnCreateVAO();
	model->vao->Initialize();
	model->vao->Bind();

	VertexBufferObject *vbo = pfnCreateVBO();
	vbo->Initialize(2);
	vbo->AddVBO(&vertices[0],	vertices.size() * sizeof(vertices[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(0, 0, false, 0, 0);
	vbo->AddVBO(&uvs[0],		uvs.size() * sizeof(uvs[0]),			2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(1, 1, false, 0, 0);
	vbo->AddVBO(&normals[0],	normals.size() * sizeof(normals[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(2, 2, false, 0, 0);
	vbo->AddVBO(&tangents[0],	tangents.size() * sizeof(tangents[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(3, 3, false, 0, 0);
	vbo->AddIBO(&indices[0],		indices.size() * sizeof(unsigned int), DRAW_STATIC); // 3 and SIZE_FLOAT are arbitrary

	model->vao->Unbind();

	vertices.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	indices.clear();

	importer.FreeScene();
}

void SModel::LoadPreparedModel3Ds() {
}

void SModel::Draw() {
	for (size_t i = 0; i < models.size(); i++)
		DrawModel3D(&models[i]);
}

void SModel::DrawModel3D(CModel *model) {
	model->vao->Bind();
	for (size_t i = 0; i < model->meshes.size(); i++) {
		model->materials[model->meshes[i].MaterialIndex]->tex->Bind();
		engine.graphicsWrapper->DrawBaseVertex((void*)(sizeof(unsigned int) * model->meshes[i].BaseIndex), model->meshes[i].BaseVertex, model->meshes[i].NumIndices);
	}
	model->vao->Unbind();
}