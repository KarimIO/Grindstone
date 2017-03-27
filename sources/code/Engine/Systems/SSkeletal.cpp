#include "SSkeletal.h"
#include "Core/Engine.h"
#include "Core/GraphicsDLLPointer.h"

void SSkeletal::InitMaterials(const aiScene* scene, std::string Dir, std::vector<Material *> &materials)
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
		materials[i] = newMat;
	}
}

void SSkeletal::InitBones(const aiMesh *paiMesh,
	std::vector<glm::tvec4<unsigned char, glm::highp>>& boneID,
	std::vector<glm::vec4>& boneWeights)
{
	/*for (unsigned int i = 0; i < paiMesh->mNumBones; i++) {
		unsigned int BoneIndex = 0;
		std::string BoneName(paiMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		m_BoneMapping[BoneName] = BoneIndex;
		m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;

		for (uint j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
			uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}*/
}

void SSkeletal::InitMesh(const aiMesh *paiMesh,
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

void SSkeletal::LoadFile(const char * szPath, CSkeletal * model) {
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

	InitMaterials(pScene, szPath, model->materials);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> uvs;
	std::vector<glm::tvec4<unsigned char, glm::highp>> boneIDs;
	std::vector<glm::vec4> boneWeights;
	std::vector<unsigned int> indices;

	vertices.reserve(NumVertices);
	normals.reserve(NumVertices);
	tangents.reserve(NumVertices);
	uvs.reserve(NumVertices);
	indices.reserve(NumIndices);

	for (size_t i = 0; i < pScene->mNumMeshes; i++) {
		InitMesh(pScene->mMeshes[i], vertices, normals, tangents, uvs, indices);
		InitBones(pScene->mMeshes[i], boneIDs, boneWeights);
	}

	model->vao = pfnCreateVAO();
	model->vao->Initialize();
	model->vao->Bind();

	VertexBufferObject *vbo = pfnCreateVBO();
	vbo->Initialize(5);
	vbo->AddVBO(&vertices[0], vertices.size() * sizeof(vertices[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(0, 0, false, 0, 0);
	vbo->AddVBO(&uvs[0], uvs.size() * sizeof(uvs[0]), 2, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(1, 1, false, 0, 0);
	vbo->AddVBO(&normals[0], normals.size() * sizeof(normals[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(2, 2, true, 0, 0);
	vbo->AddVBO(&tangents[0], tangents.size() * sizeof(tangents[0]), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(3, 3, true, 0, 0);
	vbo->AddVBO(&boneIDs[0], boneIDs.size() * sizeof(boneIDs[0]), 4, SIZE_UNSIGNED_BYTE, DRAW_STATIC);
	vbo->IBind(4, 4, 0, 0);
	vbo->AddVBO(&boneWeights[0], boneWeights.size() * sizeof(boneWeights[0]), 4, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(5, 5, true, 0, 0);
	vbo->AddIBO(&indices[0], indices.size() * sizeof(unsigned int), DRAW_STATIC); // 3 and SIZE_FLOAT are arbitrary

	model->vao->Unbind();

	vertices.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	indices.clear();

	importer.FreeScene();
}

void SSkeletal::CalculateAnimations(unsigned int modelID, unsigned int refID, std::vector<JointTransform>& joints) {
	CRenderSkeletal *renderable = &references[refID];
	unsigned int animationID = renderable->animationID;
	float animationTime = renderable->animationTime;

	CSkeletal *model = &models[modelID];
	Animation anim = model->animations[animationID];
	
	float timeSpan;
	unsigned int keyID = 0;
	for (unsigned int i = 0; i < anim.keyframes.size(); i++) {
		if (animationTime > anim.keyframes[i].time) {
			keyID = i;
			break;
		}
	}

	//timeSpan = anim.keyframes[keyID].time + anim.keyframes[keyID].time;
	joints = anim.keyframes[keyID].transforms;
}
