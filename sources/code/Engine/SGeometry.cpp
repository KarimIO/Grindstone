#include "Engine.h"

#include "SGeometry.h"
#include "GraphicsDLLPointer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "gl3w.h"

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
	//materials.resize(pScene->mNumMaterials);
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
		model->meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		model->meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		model->meshes[i].BaseVertex = NumVertices;
		model->meshes[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += model->meshes[i].NumIndices;
	}


	//std::cout << "Mesh Materials: " << path << "\n";
	//InitMaterials(pScene, pFile, materialType);

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

	std::cout << "Mesh Data: " << szPath << "\n";

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
	/*vbo->AddVBO(false, &uvs[0],			uvs.size() * sizeof(uvs[0]),			2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(1, 1, false, 0, 0);
	vbo->AddVBO(false, &normals[0],		normals.size() * sizeof(normals[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(2, 2, false, 0, 0);
	vbo->AddVBO(false, &tangents[0],	tangents.size() * sizeof(vertices[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(3, 3, false, 0, 0);*/
	vbo->AddIBO(&indices[0],		indices.size() * sizeof(unsigned int), DRAW_STATIC); // 3 and SIZE_FLOAT are arbitrary

	model->vao->Unbind();

#if 0
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(NUM_VBs, m_Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_VB]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(VERTEX_VB_LOCATION);
	glVertexAttribPointer(VERTEX_VB_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[UV_VB]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(uvs[0]), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(UV_VB_LOCATION);
	glVertexAttribPointer(UV_VB_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_VB_LOCATION);
	glVertexAttribPointer(NORMAL_VB_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TANGENT_VB]);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(tangents[0]), &tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TANGENT_VB_LOCATION);
	glVertexAttribPointer(TANGENT_VB_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_VB]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
#endif

	vertices.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	indices.clear();

	importer.FreeScene();

	/*GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}*/
}

void SModel::LoadPreparedModel3Ds() {
}

void SModel::Draw() {
	for (size_t i = 0; i < models.size(); i++)
		DrawModel3D(&models[i]);
}

void SModel::DrawModel3D(CModel *model) {
	model->vao->Bind();
	//for (size_t i = 0; i < model->references.size(); i++) {
	for (size_t i = 0; i < model->meshes.size(); i++) {
		engine.graphicsWrapper->DrawBaseVertex((void*)(sizeof(unsigned int) * model->meshes[i].BaseIndex), model->meshes[i].BaseVertex, model->meshes[i].NumIndices);
		//engine.graphicsWrapper->DrawBaseVertex(model->meshes[i].BaseIndex, model->meshes[i].BaseVertex, model->meshes[i].NumIndices);
	}
	model->vao->Unbind();
}