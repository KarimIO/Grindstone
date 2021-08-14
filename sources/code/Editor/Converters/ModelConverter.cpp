#include "ModelConverter.hpp"
#include "MaterialCreator.hpp"
#include "TextureConverter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <chrono>

#include "Common/Formats/Model.hpp"

using namespace Grindstone::Converters;

void PushVertex3dToVector(std::vector<float>& targetVector, const aiVector3D* aiVertex) {
	targetVector.push_back(aiVertex->x);
	targetVector.push_back(aiVertex->y);
	targetVector.push_back(aiVertex->z);
}

void ModelConverter::ConvertMaterials() {
	if (!scene->HasMaterials()) {
		return;
	}

	outputData.materialNames.resize(scene->mNumMaterials);
	uint32_t materialCount = scene->mNumMaterials;
	aiMaterial **materials = scene->mMaterials;

	std::string finalDir = path;
	finalDir = finalDir.substr(0, finalDir.find_last_of('/') + 1);

	std::string folderName = "";
	std::string baseOutputPath = "../assets/materials/" + folderName + "/";
	/*if (!CreateFolder(outPath.c_str())) {
		outPath = "../assets/materials/";
	}*/

	aiMaterial *pMaterial;
	aiString Path;
	for (uint32_t i = 0; i < materialCount; i++) {
		StandardMaterialCreateInfo newMaterial;
		newMaterial.albedoPath = "";
		newMaterial.normalPath = "";
		newMaterial.specularPath = "";
		newMaterial.roughnessPath = "";
		pMaterial = materials[i];

		aiString name;
		pMaterial->Get(AI_MATKEY_NAME, name);

		if (name.length == 0) {
			name = "Material_" + std::to_string(i);
		}

		ConvertTexture(pMaterial, aiTextureType_DIFFUSE, baseOutputPath, newMaterial.albedoPath);
		ConvertTexture(pMaterial, aiTextureType_NORMALS, baseOutputPath, newMaterial.normalPath);
		ConvertTexture(pMaterial, aiTextureType_AMBIENT, baseOutputPath, newMaterial.specularPath);
		ConvertTexture(pMaterial, aiTextureType_SHININESS, baseOutputPath, newMaterial.roughnessPath);

		aiColor4D diffuse_color;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color)) {
			memcpy(&newMaterial.albedoColor, &diffuse_color, sizeof(float) * 4);
		}

		aiColor4D metalness;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &metalness)) {
			newMaterial.metalness = metalness.r;
		}

		aiColor4D roughness;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_AMBIENT, &roughness)) {
			newMaterial.roughness = roughness.r;
		}

		std::string sanname = name.C_Str(); //sanitizeFileName(name.C_Str());

		std::string outputMaterialPath = baseOutputPath + sanname + ".gmat";
		outputData.materialNames[i] = outputMaterialPath;

		CreateStandardMaterial(newMaterial, outputMaterialPath);
	}
}

void ModelConverter::ConvertTexture(aiMaterial* pMaterial, aiTextureType type, std::string baseOutputPath, std::string& outPath) {
	if (pMaterial->GetTextureCount(type) > 0) {
		aiString aiPath;
		if (pMaterial->GetTexture(type, 0, &aiPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string fullPath = aiPath.data;
			// switchSlashes(FullPath);
			std::string name = fullPath.substr(fullPath.find_last_of("/") + 1);
			// name = SwapExtension(name, "dds");
			std::string finaloutpath = baseOutputPath + name;
			Grindstone::Converters::ImportTexture(/*finalDir +*/ fullPath.c_str());
			outPath = name;
		}
	}
}

void ModelConverter::InitSubmeshes() {
	auto& vertexCount = outputData.vertexCount;
	auto& indexCount = outputData.indexCount;

	outputData.meshes.resize(scene->mNumMeshes);
	for (unsigned int meshIterator = 0; meshIterator < scene->mNumMeshes; ++meshIterator) {
		Submesh& mesh = outputData.meshes[meshIterator];
		aiMesh* aiMesh = scene->mMeshes[meshIterator];

		mesh.indexCount = aiMesh->mNumFaces * 3;
		mesh.baseVertex = vertexCount;
		mesh.baseIndex = indexCount;
		mesh.materialIndex = aiMesh->mMaterialIndex;

		vertexCount += aiMesh->mNumVertices;
		indexCount += mesh.indexCount;
	}

	outputData.vertexArray.position.reserve(vertexCount * 3);
	outputData.vertexArray.normal.reserve(vertexCount * 3);
	outputData.vertexArray.tangent.reserve(vertexCount * 3);
	outputData.indices.reserve(indexCount);
}

void ModelConverter::ProcessVertices() {
	for (unsigned int meshIterator = 0; meshIterator < scene->mNumMeshes; meshIterator++) {
		const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		auto mesh = scene->mMeshes[meshIterator];

		for (unsigned int vertexIterator = 0; vertexIterator < mesh->mNumVertices; vertexIterator++) {
			const aiVector3D* aiPos = &(mesh->mVertices[vertexIterator]);
			const aiVector3D* aiNormal = &(mesh->mNormals[vertexIterator]);
			const aiVector3D* aiTangent = &(mesh->mTangents[vertexIterator]);
			const aiVector3D* aiTexCoord = mesh->HasTextureCoords(0)
				? &(mesh->mTextureCoords[0][vertexIterator])
				: &Zero3D;

			auto& vertexArray = outputData.vertexArray;
			PushVertex3dToVector(vertexArray.position, aiPos);
			PushVertex3dToVector(vertexArray.normal, aiNormal);
			PushVertex3dToVector(vertexArray.tangent, aiTangent);
			// PushVertex2d({ pTexCoord->x, pTexCoord->y});

			// const float pos[3]{ pPos->x, pPos->y, pPos->z };
			// bounding_shape_->TestBounding(pos);
		}

		for (unsigned int faceIterator = 0; faceIterator < mesh->mNumFaces; faceIterator++) {
			const aiFace& face = mesh->mFaces[faceIterator];
			outputData.indices.push_back(face.mIndices[0]);
			outputData.indices.push_back(face.mIndices[1]);
			outputData.indices.push_back(face.mIndices[2]);
		}
	}
}

void ModelConverter::ProcessNodeTree(aiNode* node) {
	node->mMeshes;

	for (unsigned int childIterator = 0; childIterator < node->mNumChildren; ++childIterator) {
		ProcessNodeTree(node->mChildren[childIterator]);
	}
}

void ModelConverter::Convert(const char* path) {
	this->path = path;
	Assimp::Importer importer;
	scene = importer.ReadFile(
		path,
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_Triangulate
	);

	if (!scene) {
		throw std::runtime_error(importer.GetErrorString());
	}

	ProcessNodeTree(scene->mRootNode);
	InitSubmeshes();
	// ConvertMaterials();
	ProcessVertices();

	importer.FreeScene();

	OutputPrefabs();
	OutputMeshes();
}

void ModelConverter::OutputPrefabs() {

}

void ModelConverter::OutputMeshes() {
	std::string meshOutputPath = path.substr(0, path.find_last_of('.')) + ".gmf";

	auto meshCount = outputData.meshes.size();

	Grindstone::Formats::Model::Header::V1 outFormat;
	outFormat.totalFileSize = sizeof(outFormat) + 
		meshCount * sizeof(Submesh) +
		outputData.vertexArray.position.size() * sizeof(float) +
		outputData.vertexArray.normal.size() * sizeof(float) +
		outputData.vertexArray.tangent.size() * sizeof(float) +
		outputData.indices.size() * sizeof(uint32_t);
	outFormat.hasVertexPositions = true;
	outFormat.hasVertexNormals = true;
	outFormat.hasVertexTangents = true;
	outFormat.vertexCount = static_cast<uint64_t>(outputData.vertexCount);
	outFormat.indexCount = static_cast<uint64_t>(outputData.indexCount);
	outFormat.meshCount = static_cast<uint32_t>(meshCount);

	std::ofstream output(meshOutputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + meshOutputPath);
	}

	//  - Output File MetaData
	output.write("GMF", 3);

	//	- Output Header
	output.write(reinterpret_cast<const char*> (&outFormat), sizeof(Grindstone::Formats::Model::Header::V1));

	//	- Output Meshes
	output.write(reinterpret_cast<const char*> (outputData.meshes.data()), meshCount * sizeof(Submesh));

	OutputVertexArray(output, outputData.vertexArray.position);
	OutputVertexArray(output, outputData.vertexArray.normal);
	OutputVertexArray(output, outputData.vertexArray.tangent);

	// - Output Indices
	output.write(reinterpret_cast<const char*> (outputData.indices.data()), outputData.indices.size() * sizeof(uint16_t));

	output.close();
}

void ModelConverter::OutputVertexArray(std::ofstream& output, std::vector<float>& vertexArray) {
	output.write(
		reinterpret_cast<const char*> (vertexArray.data()),
		vertexArray.size() * sizeof(float)
	);
}

void Grindstone::Converters::ImportModel(const char* path) {
	ModelConverter m;
	m.Convert(path);
}
