#include "../Core/Engine.h"

#include "SGeometry.h"
#include "../Core/TextureManager.h"
#include "../Core/GraphicsDLLPointer.h"

struct UniformBuffer {
	glm::mat4 pvmMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
} ubo;

std::string CModel::getName() {
	return name;
}

void SModel::PreloadModel3D(const char * szPath, size_t renderID) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == szPath) {
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	models.push_back(CModel());
	CModel *model = &models.back();
	model->references.push_back((unsigned int)renderID);
	model->name = szPath;
	unloadedModelIDs.push_back((unsigned int)(models.size() - 1));
}

void SModel::LoadPreloaded() {
	for (size_t i = 0; i < unloadedModelIDs.size(); i++) {
		CModel *model = &models[unloadedModelIDs[i]];
		LoadModel3DFile(model->getName().c_str(), model);
	}
}

void SModel::LoadModel3D(const char * szPath, size_t renderID) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == szPath) {
			renderID = models[i].references.size();
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	renderID = 0;
	models.push_back(CModel());
	CModel *model = &models.back();
	model->references.push_back((unsigned int)renderID);
	model->name = szPath;
	LoadModel3DFile(szPath, model);
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

void SModel::InitMaterials(const aiScene* scene, std::string Dir, CModel *model)
{
	std::string finalDir = Dir;
	finalDir = finalDir.substr(0, finalDir.find_last_of('/'));

	aiMaterial *pMaterial;
	aiString Path;
	for (size_t i = 0; i < scene->mNumMaterials; i++) {
		Material *newMat = new Material();
		newMat->shader = engine.shader;
		newMat->tex.resize(4);
		pMaterial = scene->mMaterials[i];

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = finalDir + "/" + Path.data;
				SwitchSlashes(FullPath);
				newMat->tex[0] = engine.textureManager.LoadTexture(FullPath, COLOR_SRGB);
			}
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0) {
			if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = finalDir + "/" + Path.data;
				SwitchSlashes(FullPath);
				newMat->tex[1] = engine.textureManager.LoadTexture(FullPath, COLOR_RGBA);
			}
		}
		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = finalDir + "/" + Path.data;
				SwitchSlashes(FullPath);
				newMat->tex[2] = engine.textureManager.LoadTexture(FullPath, COLOR_RGBA);
			}
		}
		if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = finalDir + "/" + Path.data;
				SwitchSlashes(FullPath);
				newMat->tex[3] = engine.textureManager.LoadTexture(FullPath, COLOR_RGBA);
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

	for (size_t i = 0; i < pScene->mNumMeshes; i++) {
		InitMesh(pScene->mMeshes[i], vertices, normals, tangents, uvs, indices);
	}

	model->vao = pfnCreateVAO();
	model->vao->Initialize();
	model->vao->Bind();

	VertexBufferObject *vbo = pfnCreateVBO();
	vbo->Initialize(5);
	vbo->AddVBO(&vertices[0],	vertices.size()	* sizeof(vertices[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(0, 0, false, 0, 0);
	vbo->AddVBO(&uvs[0],		uvs.size()		* sizeof(uvs[0]),		2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(1, 1, false, 0, 0);
	vbo->AddVBO(&normals[0],	normals.size()	* sizeof(normals[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(2, 2, false, 0, 0);
	vbo->AddVBO(&tangents[0],	tangents.size()	* sizeof(tangents[0]),	3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(3, 3, false, 0, 0);
	vbo->AddIBO(&indices[0],	indices.size()	* sizeof(unsigned int), DRAW_STATIC); // 3 and SIZE_FLOAT are arbitrary

	model->vao->Unbind();

	vertices.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	indices.clear();

	importer.FreeScene();
}

void SModel::AddComponent(unsigned int entID, unsigned int &target) {
	renderComponents.push_back(CRender());
	renderComponents[renderComponents.size() - 1].entityID = entID;
	target = (unsigned int)(renderComponents.size()-1);
}

void SModel::Draw(glm::mat4 projection, glm::mat4 view) {
	for (size_t i = 0; i < models.size(); i++)
		DrawModel3D(projection, view, &models[i]);
}

void SModel::DrawModel3D(glm::mat4 projection, glm::mat4 view, CModel *model) {

	ubo.viewMatrix = view;
	ubo.texLoc0 = 0;
	ubo.texLoc1 = 1;
	ubo.texLoc2 = 2;
	ubo.texLoc3 = 3;

	model->vao->Bind();
	for (size_t i = 0; i < model->references.size(); i++) {
		CRender *renderComponent = &renderComponents[model->references[i]];

		for (size_t j = 0; j < model->meshes.size(); j++) {
			unsigned char matID = model->meshes[j].MaterialIndex;
			Material *material;
			if (renderComponent->materials.size() > matID && renderComponent->materials[matID] != nullptr)
				material = renderComponent->materials[matID];
			else
				material = model->materials[matID];

			for (size_t t = 0; t < 4; t++) {
				Texture *temp = material->tex[t];
				if (temp != nullptr)
					temp->Bind(int(t));
			}

			size_t entID = renderComponent->entityID;
			unsigned int transID =  engine.entities[entID].components[COMPONENT_TRANSFORM];
			CTransform *transform = &engine.transformSystem.components[transID];
			glm::mat4 modelMatrix = transform->GetModelMatrix();
			ubo.pvmMatrix = projection * view * modelMatrix;
			ubo.modelMatrix = modelMatrix;
			ShaderProgram *shader = material->shader;
			if (shader != NULL) {
				shader->Use();
				shader->PassData(&ubo);
				shader->SetUniform4m();
				shader->SetUniform4m();
				shader->SetUniform4m();
				shader->SetInteger();
				shader->SetInteger();
				shader->SetInteger();
				shader->SetInteger();
			}
			else
				std::cout << "Shader fail: " << renderComponent->entityID << " - " << i << " - " << j << "\n";

			engine.graphicsWrapper->DrawBaseVertex(SHAPE_TRIANGLES, (void*)(sizeof(unsigned int) * model->meshes[j].BaseIndex), model->meshes[j].BaseVertex, model->meshes[j].NumIndices);
		}
	}
	model->vao->Unbind();
}

void SModel::Shutdown() {
	for (size_t i = 0; i < models.size(); i++) {
		models[i].vao->CleanupVBOs();
		pfnDeleteGraphicsPointer(models[i].vao);
	}
}
