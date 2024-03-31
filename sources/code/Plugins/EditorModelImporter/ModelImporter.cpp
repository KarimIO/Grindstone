#include "ModelImporter.hpp"
#include "MaterialImporter.hpp"
#include "TextureImporter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <chrono>

#include <EngineCore/Assets/AssetManager.hpp>
#include "Common/Formats/Model.hpp"
#include "Common/Formats/Animation.hpp"
#include "Editor/EditorManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"

using namespace Grindstone::Importers;

void PushVertex3dToVector(std::vector<float>& targetVector, const aiVector3D* aiVertex) {
	targetVector.push_back(aiVertex->x);
	targetVector.push_back(aiVertex->y);
	targetVector.push_back(aiVertex->z);
}

void PushVertex2dToVector(std::vector<float>& targetVector, const aiVector3D* aiVertex) {
	targetVector.push_back(aiVertex->x);
	targetVector.push_back(aiVertex->y);
}

glm::mat4 AiMatToGlm(aiMatrix4x4& matrix) {
	return glm::mat4(
		matrix.a1, matrix.a2, matrix.a3, matrix.a4,
		matrix.b1, matrix.b2, matrix.b3, matrix.b4,
		matrix.c1, matrix.c2, matrix.c3, matrix.c4,
		matrix.d1, matrix.d2, matrix.d3, matrix.d4
	);
}

void ModelImporter::ConvertMaterials() {
	if (!scene->HasMaterials()) {
		return;
	}

	outputData.materialNames.resize(scene->mNumMaterials);
	uint32_t materialCount = scene->mNumMaterials;
	aiMaterial **materials = scene->mMaterials;

	std::string folderName = "";
	/*if (!CreateFolder(outPath.c_str())) {
		outPath = "../assets/materials/";
	}*/

	aiMaterial *pMaterial;
	aiString Path;
	for (uint32_t i = 0; i < materialCount; i++) {
		StandardMaterialCreateInfo newMaterial;
		pMaterial = materials[i];

		aiString name;
		pMaterial->Get(AI_MATKEY_NAME, name);

		if (name.length == 0) {
			name = "Material_" + std::to_string(i);
		}

		newMaterial.albedoPath = GetTexturePath(pMaterial, aiTextureType_DIFFUSE);
		newMaterial.normalPath = GetTexturePath(pMaterial, aiTextureType_NORMALS);
		newMaterial.specularPath = GetTexturePath(pMaterial, aiTextureType_AMBIENT);
		newMaterial.roughnessPath = GetTexturePath(pMaterial, aiTextureType_SHININESS);

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

		newMaterial.materialName = name.C_Str();
		Uuid uuid = metaFile->GetOrCreateSubassetUuid(newMaterial.materialName, AssetType::Material);
		std::string uuidString = outputData.materialNames[i] = uuid.ToString();

		std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuidString;
		CreateStandardMaterial(newMaterial, outputPath);
	}
}

std::filesystem::path ModelImporter::GetTexturePath(aiMaterial* pMaterial, aiTextureType type) {
	if (pMaterial->GetTextureCount(type) > 0) {
		aiString aiPath;
		if (pMaterial->GetTexture(type, 0, &aiPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string fullPath = aiPath.data;
			return baseFolderPath / fullPath;
		}
	}

	return "";
}

void ModelImporter::InitSubmeshes(bool hasBones) {
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

	size_t vertexCountSizeT = vertexCount;
	outputData.vertexArray.position.reserve(vertexCountSizeT * 3);
	outputData.vertexArray.normal.reserve(vertexCountSizeT * 3);
	outputData.vertexArray.tangent.reserve(vertexCountSizeT * 3);
	outputData.vertexArray.texCoordArray.resize(1);
	outputData.vertexArray.texCoordArray[0].reserve(vertexCountSizeT * 2);
	size_t boneWeightCount = hasBones
		? vertexCountSizeT * NUM_BONES_PER_VERTEX
		: 0;
	outputData.vertexArray.boneIds.resize(boneWeightCount);
	outputData.vertexArray.boneWeights.resize(boneWeightCount);
	outputData.indices.reserve(indexCount);
}

void ModelImporter::ProcessVertices() {
	for (unsigned int meshIterator = 0; meshIterator < scene->mNumMeshes; meshIterator++) {
		const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

		auto mesh = scene->mMeshes[meshIterator];

		for (unsigned int vertexIterator = 0; vertexIterator < mesh->mNumVertices; vertexIterator++) {
			const aiVector3D* aiPos = &(mesh->mVertices[vertexIterator]);
			const aiVector3D* aiNormal = &(mesh->mNormals[vertexIterator]);
			const aiVector3D* aiTangent = &(mesh->mTangents[vertexIterator]);
			const aiVector3D* aiTexCoord = mesh->HasTextureCoords(0)
				? &(mesh->mTextureCoords[0][vertexIterator])
				: &zero3D;

			auto& vertexArray = outputData.vertexArray;
			PushVertex3dToVector(vertexArray.position, aiPos);
			PushVertex3dToVector(vertexArray.normal, aiNormal);
			PushVertex3dToVector(vertexArray.tangent, aiTangent);
			PushVertex2dToVector(vertexArray.texCoordArray[0], aiTexCoord);

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

// Make a list of used bones so we can remove unnecessary bones, and get offset Matrix
void ModelImporter::PreprocessBones() {
	for (unsigned int meshIterator = 0; meshIterator < scene->mNumMeshes; meshIterator++) {
		auto mesh = scene->mMeshes[meshIterator];

		for (unsigned int boneIterator = 0; boneIterator < mesh->mNumBones; boneIterator++) {
			auto bone = mesh->mBones[boneIterator];
			std::string boneName(bone->mName.data);

			tempOffsetMatrices[boneName] = AiMatToGlm(bone->mOffsetMatrix);
		}
	}
}

void ModelImporter::ProcessNodeTree(aiNode* node, uint16_t parentIndex) {
	std::string name(node->mName.data);

	uint16_t currentBone = outputData.boneCount++;
	auto boneOffsetMatrixIterator = tempOffsetMatrices.find(name);
	bool isBone = boneOffsetMatrixIterator != tempOffsetMatrices.end();
	if (isBone) {
		glm::mat4 inverseMatrix = glm::inverse(AiMatToGlm(node->mTransformation));
		glm::mat4& offsetMatrix = boneOffsetMatrixIterator->second;
		outputData.boneNames.push_back(name);
		outputData.bones.emplace_back(parentIndex, offsetMatrix, inverseMatrix);
		boneMapping[name.c_str()] = currentBone;
	}

	for (unsigned int childIterator = 0; childIterator < node->mNumChildren; ++childIterator) {
		ProcessNodeTree(node->mChildren[childIterator], currentBone);
	}
}

void ModelImporter::ProcessVertexBoneWeights() {
	for (unsigned int meshIterator = 0; meshIterator < scene->mNumMeshes; meshIterator++) {
		auto mesh = scene->mMeshes[meshIterator];

		for (unsigned int boneIterator = 0; boneIterator < mesh->mNumBones; boneIterator++) {
			auto bone = mesh->mBones[boneIterator];
			std::string boneName(bone->mName.data);
			unsigned int boneId = boneMapping[boneName];

			for (unsigned int weightIterator = 0; weightIterator < bone->mNumWeights; weightIterator++) {
				auto& weight = bone->mWeights[weightIterator];

				auto vertexId = weight.mVertexId;
				float vertexWeight = weight.mWeight;
				AddBoneData(vertexId, boneId, vertexWeight);
			}
		}
	}

	if (hasExtraWeights) {
		NormalizeBoneWeights();
	}
}

void ModelImporter::NormalizeBoneWeights() {
	for (size_t i = 0; i < outputData.vertexArray.boneWeights.size(); i += NUM_BONES_PER_VERTEX) {
		float w0 = outputData.vertexArray.boneWeights[i];
		float w1 = outputData.vertexArray.boneWeights[i + 1];
		float w2 = outputData.vertexArray.boneWeights[i + 2];
		float w3 = outputData.vertexArray.boneWeights[i + 3];
		float total = w0 + w1 + w2 + w3;

		outputData.vertexArray.boneWeights[i] = w0 / total;
		outputData.vertexArray.boneWeights[i + 1] = w1 / total;
		outputData.vertexArray.boneWeights[i + 2] = w2 / total;
		outputData.vertexArray.boneWeights[i + 3] = w3 / total;
	}
}

void ModelImporter::AddBoneData(unsigned int vertexId, unsigned int boneId, float vertexWeight) {
	unsigned int baseIndex = vertexId * NUM_BONES_PER_VERTEX;
	unsigned int lastIndex = baseIndex + NUM_BONES_PER_VERTEX;
	for (unsigned int i = baseIndex; i < lastIndex; i++) {
		if (outputData.vertexArray.boneWeights[i] == 0.0f) {
			outputData.vertexArray.boneIds[i] = boneId;
			outputData.vertexArray.boneWeights[i] = vertexWeight;
			return;
		}
	}

	// Too many boneweights - replace the smallest one
	hasExtraWeights = true;
	unsigned int lowestIndex = baseIndex;
	float lowestWeight = outputData.vertexArray.boneWeights[lowestIndex];
	for (unsigned int i = baseIndex; i < lastIndex; i++) {
		float currentWeight = outputData.vertexArray.boneWeights[i];
		if (currentWeight < lowestWeight) {
			lowestIndex = i;
			lowestWeight = currentWeight;
		}
	}

	if (lowestWeight < vertexWeight) {
		outputData.vertexArray.boneIds[lowestIndex] = boneId;
		outputData.vertexArray.boneWeights[lowestIndex] = vertexWeight;
	}
}

void ModelImporter::ProcessAnimations() {
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
		auto animation = scene->mAnimations[i];
		std::string animationName(animation->mName.data);

		double ticksPerSecond = animation->mTicksPerSecond != 0
			? animation->mTicksPerSecond
			: 25.0f;
		double duration = animation->mDuration;

		Formats::Animation::V1::Header animationHeader;
		animationHeader.animationDuration = duration;
		animationHeader.channelCount = static_cast<uint16_t>(animation->mNumChannels);
		animationHeader.ticksPerSecond = ticksPerSecond;

		std::vector<Formats::Animation::V1::Channel> channels;
		channels.resize(animation->mNumChannels);
		std::vector<Formats::Animation::V1::ChannelData> channelData;
		channels.resize(animation->mNumChannels);

		std::string subassetName = "anim-" + animationName;
		Uuid outUuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Animation);

		std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / outUuid.ToString();
		std::ofstream output(outputPath, std::ios::binary);

		if (!output.is_open()) {
			throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
		}

		//  - Output File MetaData
		output.write("GAF", 3);

		for (unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
			Formats::Animation::V1::Channel& dstChannel = channels[channelIndex];
			Formats::Animation::V1::ChannelData& dstChannelData = channelData[channelIndex];
			auto srcChannel = animation->mChannels[channelIndex];
			std::string channelName(srcChannel->mNodeName.data);
			auto boneIterator = boneMapping.find(channelName);
			bool isBone = boneIterator == boneMapping.end();

			if (isBone) {
				dstChannel.boneIndex = boneIterator->second;

				dstChannel.positionCount = srcChannel->mNumPositionKeys;
				dstChannel.rotationCount = srcChannel->mNumRotationKeys;
				dstChannel.scaleCount = srcChannel->mNumScalingKeys;

				dstChannelData.positions.reserve(srcChannel->mNumPositionKeys);
				for (unsigned int i = 0; i < srcChannel->mNumPositionKeys; ++i) {
					double time = srcChannel->mPositionKeys[i].mTime;
					aiVector3D& srcValue = srcChannel->mPositionKeys[i].mValue;
					Math::Float3 value = Math::Float3(srcValue.x, srcValue.y, srcValue.z);
					dstChannelData.positions.emplace_back(time, value);
				}

				dstChannelData.rotations.reserve(srcChannel->mNumRotationKeys);
				for (unsigned int i = 0; i < srcChannel->mNumRotationKeys; ++i) {
					double time = srcChannel->mRotationKeys[i].mTime;
					aiQuaternion& srcValue = srcChannel->mRotationKeys[i].mValue;
					Math::Quaternion value = Math::Quaternion(srcValue.x, srcValue.y, srcValue.z, srcValue.w);
					dstChannelData.rotations.emplace_back(time, value);
				}

				dstChannelData.scales.reserve(srcChannel->mNumScalingKeys);
				for (unsigned int i = 0; i < srcChannel->mNumScalingKeys; ++i) {
					double time = srcChannel->mScalingKeys[i].mTime;
					aiVector3D& srcValue = srcChannel->mScalingKeys[i].mValue;
					Math::Float3 value = Math::Float3(srcValue.x, srcValue.y, srcValue.z);
					dstChannelData.scales.emplace_back(time, value);
				}
			}
		}
	}
}

void ModelImporter::Import(std::filesystem::path& path) {
	this->path = path;
	this->baseFolderPath = this->path.parent_path();

	Assimp::Importer importer;
	scene = importer.ReadFile(
		path.string(),
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_Triangulate |
		aiProcess_FlipUVs
	);

	if (!scene) {
		throw std::runtime_error(importer.GetErrorString());
	}

	metaFile = new MetaFile(path);
	// Set to false, will check if true later.
	bool shouldImportAnimations = false;
	isSkeletalMesh = false;

	InitSubmeshes(isSkeletalMesh);
	ConvertMaterials();
	ProcessVertices();

	if (isSkeletalMesh) {
		PreprocessBones();
		ProcessNodeTree(scene->mRootNode, -1);
		ProcessVertexBoneWeights();
	}
	/*
	TODO: Process lights, etc here
	else {
		ProcessNodeTree(scene->mRootNode, -1);
	}
	*/

	if (shouldImportAnimations) {
		ProcessAnimations();
	}

	importer.FreeScene();

	OutputPrefabs();
	OutputMeshes();

	metaFile->Save();
}

void ModelImporter::OutputPrefabs() {

}

void ModelImporter::OutputMeshes() {
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Uuid outUuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Mesh3d);

	std::filesystem::path meshOutputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / outUuid.ToString();

	auto meshCount = outputData.meshes.size();

	Grindstone::Formats::Model::V1::Header outFormat;
	outFormat.totalFileSize = static_cast<uint32_t>(
		3 +
		sizeof(outFormat) + 
		meshCount * sizeof(Submesh) +
		outputData.vertexArray.position.size() * sizeof(float) +
		outputData.vertexArray.normal.size() * sizeof(float) +
		outputData.vertexArray.tangent.size() * sizeof(float) +
		outputData.vertexArray.texCoordArray[0].size() * sizeof(float) +
		outputData.indices.size() * sizeof(uint16_t)
	);
	outFormat.hasVertexPositions = true;
	outFormat.hasVertexNormals = true;
	outFormat.hasVertexTangents = true;
	outFormat.vertexUvSetCount = 1;
	outFormat.numWeightPerBone = isSkeletalMesh ? 4 : 0;
	outFormat.vertexCount = static_cast<uint64_t>(outputData.vertexCount);
	outFormat.indexCount = static_cast<uint64_t>(outputData.indexCount);
	outFormat.meshCount = static_cast<uint32_t>(meshCount);

	if (isSkeletalMesh) {
		outFormat.totalFileSize += static_cast<uint32_t>(
			outputData.vertexArray.boneIds.size() * sizeof(uint16_t) +
			outputData.vertexArray.boneWeights.size() * sizeof(float)
		);
	}

	std::ofstream output(meshOutputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + meshOutputPath.string());
	}

	//  - Output File MetaData
	output.write("GMF", 3);

	//	- Output Header
	output.write(reinterpret_cast<const char*> (&outFormat), sizeof(Grindstone::Formats::Model::V1::Header));

	//	- Output Meshes
	output.write(reinterpret_cast<const char*> (outputData.meshes.data()), meshCount * sizeof(Submesh));

	OutputVertexArray(output, outputData.vertexArray.position);
	OutputVertexArray(output, outputData.vertexArray.normal);
	OutputVertexArray(output, outputData.vertexArray.tangent);
	OutputVertexArray(output, outputData.vertexArray.texCoordArray[0]);

	if (isSkeletalMesh) {
		OutputVertexArray(output, outputData.vertexArray.boneIds);
		OutputVertexArray(output, outputData.vertexArray.boneWeights);
	}

	// - Output Indices
	output.write(reinterpret_cast<const char*> (outputData.indices.data()), outputData.indices.size() * sizeof(uint16_t));
	
	// - Output Bone Names
	for (auto& name : outputData.boneNames) {
		output.write(name.data(), name.size() + 1); // size + 1 to include null terminated character
	}

	output.close();

	Editor::Manager::GetEngineCore().assetManager->QueueReloadAsset(AssetType::Mesh3d, outUuid);
}

void ModelImporter::OutputVertexArray(std::ofstream& output, std::vector<uint16_t>& vertexArray) {
	output.write(
		reinterpret_cast<const char*> (vertexArray.data()),
		vertexArray.size() * sizeof(uint16_t)
	);
}

void ModelImporter::OutputVertexArray(std::ofstream& output, std::vector<float>& vertexArray) {
	output.write(
		reinterpret_cast<const char*> (vertexArray.data()),
		vertexArray.size() * sizeof(float)
	);
}

void Grindstone::Importers::ImportModel(std::filesystem::path& path) {
	ModelImporter modelImporter;
	modelImporter.Import(path);
}
