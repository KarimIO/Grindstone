#include "ModelConverter.h"
#include "MaterialCreator.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include "Utilities.h"

#include <map>

struct Vertex {
	float positions[3];
	float normal[3];
	float tangent[3];
	float tex_coord[2];
};

struct VertexSkeletal {
	float		positions[3];
	float		normal[3];
	float		tangent[3];
	float		tex_coord[2];
	uint16_t	bone_ids[4];
	float		bone_weights[4];
};

struct MaterialReference {
	uint8_t		renderpass = 0;
	uint8_t		pipeline = 0;
	uint16_t	material = 0;
};

struct Mesh {
	MaterialReference material;
	uint32_t num_indices = 0;
	uint32_t base_vertex = 0;
	uint32_t base_index = 0;
	void *vertex_array_object = nullptr;
};

struct ModelFormatHeader {
	uint32_t version;
	uint32_t num_meshes;
	uint64_t num_vertices;
	uint64_t num_indices;
	uint32_t num_materials;
	uint8_t num_bones;
	bool large_index;
};

void SwitchSlashes(std::string &path) {
	size_t index = 0;
	while (true) {
		// Locate the substring to replace.
		index = path.find("\\", index);
		if (index == std::string::npos) break;

		// Make the replacement.
		path.replace(index, 1, "/");

		// Advance index forward so the next iteration doesn't pick it up as well.
		index += 1;
	}
}

void InitMaterials(bool skeletalMaterials, std::string folder_name, std::string path,
	uint32_t num_materials, aiMaterial **materials, std::vector<std::string> &mat_names) {
	std::string finalDir = path;
	finalDir = finalDir.substr(0, finalDir.find_last_of('/') + 1);

	std::string outPath = "../assets/materials/" + folder_name + "/";
	if (!CreateFolder(outPath.c_str())) {
		outPath = "../assets/materials/";
	}

	aiMaterial *pMaterial;
	aiString Path;
	for (uint32_t i = 0; i < num_materials; i++) {
		StandardMaterialCreateInfo newMat;
		newMat.albedoPath = "../albedo.png";
		newMat.normalPath = "../normal.png";
		newMat.specularPath = "../dielectric.png";
		newMat.roughnessPath = "../roughness.png";
		pMaterial = materials[i];

		aiString name;
		pMaterial->Get(AI_MATKEY_NAME, name);

		if (strcmp(name.C_Str(), "") == 0) {
			name = "Material_" + std::to_string(i);
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				SwitchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				CopyFileTo(finalDir + FullPath, outPath + name);
				newMat.albedoPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0) {
			if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				SwitchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				CopyFileTo(finalDir + FullPath, outPath + name);
				newMat.normalPath = name;
			}
		}
		else if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				SwitchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				CopyFileTo(finalDir + FullPath, outPath + name);
				newMat.normalPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				SwitchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				CopyFileTo(finalDir + FullPath, outPath + name);
				newMat.specularPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				SwitchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				CopyFileTo(finalDir + FullPath, outPath + name);
				newMat.roughnessPath = name;
			}
		}

		std::string outMat = outPath + name.C_Str();
		mat_names[i] = outMat;

		std::cout << "\tOutputting material: " << outMat << ".gjm and .gbm\n";

		std::string shader = skeletalMaterials ? "../shaders/skeletal" : "../shaders/standard";
		CreateMaterialJsonFile(newMat, outMat + ".gjm");
		CreateMaterialBinaryFile(newMat, outMat + ".gbm");
	}
}

void InitMesh(const aiMesh *paiMesh,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices) {
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
		const aiVector3D* pTangent = paiMesh->HasTangentsAndBitangents() ? &(paiMesh->mTangents[i]) : &Zero3D;
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		vertices.push_back({ { pPos->x, pPos->y, pPos->z },{ pNormal->x, pNormal->y, pNormal->z },{ -pTangent->x, -pTangent->y, -pTangent->z },{ pTexCoord->x, pTexCoord->y } });
	}

	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}
}

std::map<std::string, uint16_t> m_BoneMapping;
uint16_t NumBones = 0;

#define NUM_IDs 4

void AddBoneData(uint32_t bone_id, float weight, VertexSkeletal &vertex) {
	for (uint16_t i = 0; i < NUM_IDs; i++) {
		if (vertex.bone_weights[i] == 0.0f) {
			vertex.bone_ids[i] = bone_id;
			vertex.bone_weights[i] = weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

void InitMesh(const aiMesh *paiMesh,
	std::vector<VertexSkeletal>& vertices,
	std::vector<uint32_t>& indices,
	std::vector<aiMatrix4x4>& boneOffsetMatrices,
	uint32_t baseVertex) {
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
		const aiVector3D* pTangent = paiMesh->HasTangentsAndBitangents() ? &(paiMesh->mTangents[i]) : &Zero3D;
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		vertices.push_back({ { pPos->x, pPos->y, pPos->z },{ pNormal->x, pNormal->y, pNormal->z },{ -pTangent->x, -pTangent->y, -pTangent->z },{ pTexCoord->x, pTexCoord->y } });
	}

	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}

	for (uint32_t j = 0; j < paiMesh->mNumBones; j++) {
		aiBone *bone = paiMesh->mBones[j];
		std::string boneName = bone->mName.C_Str();
		uint16_t boneID = 0;
		if (m_BoneMapping.find(boneName) == m_BoneMapping.end()) {
			m_BoneMapping[boneName] = NumBones++;
			boneID = NumBones;
		}
		else
			boneID = m_BoneMapping[boneName];

		boneOffsetMatrices[boneID] = bone->mOffsetMatrix;

		for (uint16_t k = 0; k < bone->mNumWeights; k++) {
			aiVertexWeight *weight = &bone->mWeights[k];
			uint32_t VertexID = baseVertex + weight->mVertexId;
			AddBoneData(boneID, weight->mWeight, vertices[VertexID]);
		}
	}
}

bool ModelConverter(std::string inputPath) {
	auto t_start = std::chrono::high_resolution_clock::now();

	SwitchSlashes(inputPath);
	std::string fileName = inputPath.substr(0, inputPath.find_last_of("."));;
	fileName = fileName.substr(fileName.find_last_of("/") + 1);
	std::string outputPath = "../assets/models/" + fileName + ".gmf";
	std::cout << "Loading model: " << inputPath << ".\n";

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(inputPath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_PreTransformVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!pScene) {
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	std::cout << "Model Loaded! Parsing.\n";

	// TODO: Fix this detection
	bool isStaticMesh = true;
	

	// TODO: Break into many models (incl. Transforms, Lights, etc) + create prefab?
	if (isStaticMesh) {
		// Static Model
		unsigned int NumVertices = 0;
		unsigned int NumIndices = 0;

		std::vector<Mesh> meshes;
		meshes.resize(pScene->mNumMeshes);
		for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
			meshes[i].num_indices = pScene->mMeshes[i]->mNumFaces * 3;
			meshes[i].base_vertex = NumVertices;
			meshes[i].base_index = NumIndices;
			meshes[i].material.material = pScene->mMeshes[i]->mMaterialIndex;

			NumVertices += pScene->mMeshes[i]->mNumVertices;
			NumIndices += meshes[i].num_indices;
		}

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		vertices.reserve(NumVertices);
		indices.reserve(NumIndices);

		std::vector<std::string> matNames;
		matNames.resize(pScene->mNumMaterials);
		if (pScene->HasMaterials()) {
			InitMaterials(false, fileName, inputPath, pScene->mNumMaterials, pScene->mMaterials, matNames);
		}

		for (size_t i = 0; i < pScene->mNumMeshes; i++) {
			InitMesh(pScene->mMeshes[i], vertices, indices);
		}

		importer.FreeScene();

		ModelFormatHeader outFormat;
		outFormat.version = 1;
		outFormat.large_index = false;
		outFormat.num_vertices = vertices.size();
		outFormat.num_indices = indices.size();
		outFormat.num_meshes = meshes.size();
		outFormat.num_materials = matNames.size();
		outFormat.num_bones = 0;

		std::cout << "Model Parsed! Outputting.\n";

		std::ofstream output(outputPath, std::ios::binary);
		output.write(reinterpret_cast<const char*> (&outFormat), sizeof(ModelFormatHeader));
		output.write(reinterpret_cast<const char*> (meshes.data()), meshes.size() * sizeof(Mesh));
		output.write(reinterpret_cast<const char*> (vertices.data()), vertices.size() * sizeof(Vertex));
		output.write(reinterpret_cast<const char*> (indices.data()), indices.size() * sizeof(uint32_t));
		for (const auto &matName : matNames) {
			output << matName << '\0';
		}
		output.close();
	}
	else {
		// Skeletal Model
		unsigned int NumVertices = 0;
		unsigned int NumIndices = 0;

		std::vector<Mesh> meshes;
		meshes.resize(pScene->mNumMeshes);

		for (uint32_t i = 0; i < pScene->mNumMeshes; i++) {
			aiMesh *mesh = pScene->mMeshes[i];

			meshes[i].num_indices = mesh->mNumFaces * 3;
			meshes[i].base_vertex = NumVertices;
			meshes[i].base_index = NumIndices;
			meshes[i].material.material = mesh->mMaterialIndex;

			NumVertices += mesh->mNumVertices;
			NumIndices += meshes[i].num_indices;
		}

		std::vector<VertexSkeletal> vertices;
		std::vector<unsigned int> indices;

		vertices.reserve(NumVertices);
		indices.reserve(NumIndices);

		std::vector<std::string> material_names;
		material_names.resize(pScene->mNumMaterials);
		if (pScene->HasMaterials()) {
			InitMaterials(true, fileName, inputPath, pScene->mNumMaterials, pScene->mMaterials, material_names);
		}

		std::vector<aiMatrix4x4> offset_matrices;
		std::vector<std::string> bone_names;
		for (size_t i = 0; i < pScene->mNumMeshes; i++) {
			InitMesh(pScene->mMeshes[i], vertices, indices, offset_matrices, meshes[i].base_vertex);
		}

		for (auto &bone_map : m_BoneMapping) {
			std::cout << bone_map.first << " - " << bone_map.second << '\n';
		}

		importer.FreeScene();

		ModelFormatHeader outFormat;
		outFormat.version = 1;
		outFormat.large_index = false;
		outFormat.num_vertices = vertices.size();
		outFormat.num_indices = indices.size();
		outFormat.num_meshes = meshes.size();
		outFormat.num_materials = material_names.size();
		outFormat.num_bones = NumBones;

		std::cout << "Model Parsed! Outputting.\n";

		std::ofstream output(outputPath, std::ios::binary);
		output.write(reinterpret_cast<const char*> (&outFormat), sizeof(ModelFormatHeader));
		output.write(reinterpret_cast<const char*> (meshes.data()), meshes.size() * sizeof(Mesh));
		output.write(reinterpret_cast<const char*> (vertices.data()), vertices.size() * sizeof(VertexSkeletal));
		output.write(reinterpret_cast<const char*> (indices.data()), indices.size() * sizeof(uint32_t));
		output.write(reinterpret_cast<const char*> (offset_matrices.data()), offset_matrices.size() * sizeof(aiMatrix4x4));

		for (const auto &material_name : material_names) {
			output << material_name << '\0';
		}

		for (const auto &boneName : bone_names) {
			output << boneName << '\0';
		}

		output.close();
	}

	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";
	std::cout << "Model Outputted to: " << outputPath << "!\n";
}

bool LoadModelFile(std::string inputPath) {
	auto t_start = std::chrono::high_resolution_clock::now();

	std::ifstream input(inputPath, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		std::cerr << "Failed to open file: " << inputPath.c_str() << "!\n";
		return false;
	}

	std::cout << "Model reading from: " << inputPath << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	ModelFormatHeader inFormat;
	void *offset = buffer.data();
	memcpy(&inFormat, offset, sizeof(ModelFormatHeader));

	std::vector<Mesh> remeshes;
	remeshes.resize(inFormat.num_meshes);
	std::vector<Vertex> revertices;
	revertices.resize(inFormat.num_vertices);
	std::vector<unsigned int> reindices;
	reindices.resize(inFormat.num_indices);
	std::vector<std::string> materialNames;
	materialNames.resize(inFormat.num_materials);

	offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
	uint32_t size = inFormat.num_meshes * sizeof(Mesh);
	memcpy(&remeshes[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.num_vertices * sizeof(Vertex);
	memcpy(&revertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.num_indices * sizeof(unsigned int);
	memcpy(&reindices[0], offset, size);
	offset = static_cast<char*>(offset) + inFormat.num_indices * sizeof(unsigned int);

	char *words = (char *)offset;
	for (int i = 0; i < inFormat.num_materials; i++) {
		materialNames[i] = words;
		LoadMaterial(words);
		words = strchr(words, '\0')+1;
	}

	input.close();

	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";

	std::cout << "Model read!\n";
}
