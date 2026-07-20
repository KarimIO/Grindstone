#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <Common/Formats/Animation.hpp>
#include <Common/Formats/Rig.hpp>
#include <EditorCommon/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>

#include <Grindstone.Editor.ModelImporter/include/ModelImporter.hpp>
#include <Grindstone.Editor.ModelImporter/include/ModelMaterialImporter.hpp>

using namespace Grindstone::Editor::Importers;

struct BoneInfo {
	aiNode* node = nullptr;
	uint16_t boneIndex;
	uint16_t parentIndex;
	glm::mat4 offsetMatrix;
	glm::mat4 inverseModelMatrix;
};

const uint16_t NUM_BONES_PER_VERTEX = 4;

class ModelImporter {
public:
	void Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path);
private:
	struct Prefab {

	};

	struct ScenePrefab {

	};

	struct Submesh {
		uint32_t indexCount = 0;
		uint32_t baseVertex = 0;
		uint32_t baseIndex = 0;
		uint32_t materialIndex = UINT32_MAX;
	};

private:
	// Members
	Grindstone::Assets::AssetManager* assetManager = nullptr;
	Grindstone::Editor::AssetRegistry* assetRegistry = nullptr;
	std::filesystem::path path;
	std::filesystem::path baseFolderPath;
	const aiScene* scene = nullptr;
	bool hasExtraWeights = false;
	bool isSkeletalMesh = false;

	struct OutputMesh {
		std::string name;
		Grindstone::Formats::Model::V1::BoundingData boundingData;
		uint32_t vertexCount = 0;
		struct VertexArray {
			std::vector<float> position;
			std::vector<float> normal;
			std::vector<float> tangent;
			std::vector<uint16_t> boneIds; // For animation
			std::vector<float> boneWeights; // For animation
			std::vector<std::vector<float>> texCoordArray;
		} vertexArray;
		std::vector<uint16_t> indices;
		std::vector<Submesh> submeshes;
	};

	struct OutputNode {
		size_t parentNode = SIZE_MAX;
		std::string name;
		aiVector3D position;
		aiVector3D scale = aiVector3D(1.0f, 1.0f, 1.0f);
		aiQuaternion rotation;
		size_t meshIndex = SIZE_MAX;
		aiLight* lightData = nullptr;
		aiCamera* cameraData = nullptr;
	};

	Grindstone::Editor::MetaFile metaFile;
	std::vector<std::string> outputMaterialUuids;
	std::vector<std::string> outputMeshUuids;
	std::vector<OutputMesh> outputMeshes;
	std::vector<OutputNode> outputNodes;

	void ProcessLight(aiLight* light);
	void ProcessCamera(aiCamera* camera);
	void ProcessNodeTree(aiNode* node, size_t parentIndex);
	void ProcessMaterial(size_t materialIndex, aiMaterial* inputMaterial);
	void ProcessVertexBoneWeights(const aiMesh* inputMesh, const std::map<std::string, uint16_t>& nameToBoneIndexMap, OutputMesh& outputMesh);
	void NormalizeBoneWeights(OutputMesh& outputMesh);
	void ProcessAnimation(aiAnimation* animation, const std::map<std::string, uint16_t>& nameToBoneIndexMap);
	void AddBoneData(OutputMesh& outputMesh, unsigned int vertexId, unsigned int boneId, float vertexWeight);
	void InitSubmeshes(aiMesh* inputMesh, OutputMesh& outputMesh, bool hasBones);
	void ProcessVertices(aiMesh* inputMesh, OutputMesh& outputMesh);
	void WritePrefab();
	void WriteMesh(size_t index, const OutputMesh& mesh);
};

static void OutputVertexArray(std::ofstream& output, const std::vector<uint16_t>& vertexArray) {
	output.write(
		reinterpret_cast<const char*> (vertexArray.data()),
		vertexArray.size() * sizeof(uint16_t)
	);
}

static void OutputVertexArray(std::ofstream& output, const std::vector<float>& vertexArray) {
	output.write(
		reinterpret_cast<const char*> (vertexArray.data()),
		vertexArray.size() * sizeof(float)
	);
}

static void PushVertex3dToVector(std::vector<float>& targetVector, const aiVector3D* aiVertex) {
	targetVector.push_back(aiVertex->x);
	targetVector.push_back(aiVertex->y);
	targetVector.push_back(aiVertex->z);
}

static void PushVertex2dToVector(std::vector<float>& targetVector, const aiVector3D* aiVertex) {
	targetVector.push_back(aiVertex->x);
	targetVector.push_back(aiVertex->y);
}

static glm::mat4 AiMatToGlm(aiMatrix4x4& matrix) {
	return glm::mat4(
		matrix.a1, matrix.a2, matrix.a3, matrix.a4,
		matrix.b1, matrix.b2, matrix.b3, matrix.b4,
		matrix.c1, matrix.c2, matrix.c3, matrix.c4,
		matrix.d1, matrix.d2, matrix.d3, matrix.d4
	);
}

static std::filesystem::path GetTexturePath(const std::filesystem::path& baseFolderPath, aiMaterial* material, aiTextureType type) {
	if (material->GetTextureCount(type) > 0) {
		aiString aiPath;
		if (material->GetTexture(type, 0, &aiPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			std::string fullPath = aiPath.data;
			return baseFolderPath / fullPath;
		}
	}

	return "";
}

template<typename T>
static size_t GetVectorSize(const std::vector<T>& data) {
	return data.size() * sizeof(data[0]);
}

template<typename T>
static void OutputVector(std::ofstream& output, const std::vector<T>& data, size_t vectorSize) {
	output.write(reinterpret_cast<const char*>(data.data()), vectorSize);
}

static void WriteRig(
	const std::filesystem::path& outputPath,
	const std::vector<Grindstone::Formats::Rig::V1::Bone> bones,
	const std::vector<char> stringBlockBuffer
) {
	std::ofstream output(outputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
	}

	const size_t headerSize = sizeof(Grindstone::Formats::Rig::V1::Header);
	const size_t boneVecSize = GetVectorSize(bones);
	const size_t stringBlockLength = stringBlockBuffer.size();

	const Grindstone::Formats::Rig::V1::Header header{
		.totalFileSize = static_cast<uint64_t>(4 + headerSize + boneVecSize + stringBlockLength),
		.version = Grindstone::Formats::Animation::V1::version,
		.bonesCount = static_cast<uint16_t>(bones.size()),
		.boneDataOffset = 4 + headerSize,
		.stringBlockSize = stringBlockLength,
		.stringBlockOffset = 4 + headerSize + boneVecSize
	};

	//  - Output File MetaData
	output.write(Grindstone::Formats::Rig::V1::magicCode, Grindstone::Formats::Rig::V1::magicSize);
	output.write(reinterpret_cast<const char*>(&header), headerSize);
	OutputVector(output, bones, boneVecSize);
	output.write(stringBlockBuffer.data(), stringBlockBuffer.size());
}

static void ProcessSkeletonRig(Grindstone::Editor::MetaFile& metaFile, const aiSkeleton* skeleton) {
	Grindstone::Editor::AssetRegistry& assetRegistry = Grindstone::Editor::Manager::GetInstance().GetAssetRegistry();
	std::string rigName(skeleton->mName.data);

	size_t stringBlockLength = 0;

	std::string subassetName = "rig-" + rigName;
	Grindstone::Uuid outUuid = metaFile.GetOrCreateSubassetUuid(subassetName, Grindstone::AssetType::Rig);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / outUuid.ToString();

	std::vector<Grindstone::Formats::Rig::V1::Bone> bones;
	std::vector<char> stringBlockBuffer;

	for (unsigned int boneIterator = 0; boneIterator < skeleton->mNumBones; boneIterator++) {
		aiSkeletonBone* bone = skeleton->mBones[boneIterator];
		size_t stringLength = strlen(bone->mNode->mName.data);
		stringBlockLength += stringLength + 1;
	}

	bones.reserve(skeleton->mNumBones);
	stringBlockBuffer.resize(stringBlockLength);

	size_t stringBlockBufferOffset = 0;
	for (unsigned int boneIterator = 0; boneIterator < skeleton->mNumBones; boneIterator++) {
		aiSkeletonBone* bone = skeleton->mBones[boneIterator];

		Grindstone::Formats::Rig::V1::Bone& dstBone = bones.emplace_back();
		dstBone.localMatrix = AiMatToGlm(bone->mOffsetMatrix);

		size_t stringLengthWithNull = strlen(bone->mNode->mName.data) + 1;
		errno_t cpyRes = strcpy_s(stringBlockBuffer.data() + stringBlockBufferOffset, stringLengthWithNull, bone->mNode->mName.data);
		GS_ASSERT(cpyRes == 0);

		stringBlockBufferOffset += stringLengthWithNull;
	}

	WriteRig(outputPath, bones, stringBlockBuffer);
}

static void ProcessMeshRig(
	Grindstone::Editor::MetaFile& metaFile,
	const std::map<std::string, uint16_t>& nameToBoneIndexMap,
	const std::vector<BoneInfo>& orderedSkinnedBones
) {
	Grindstone::Editor::AssetRegistry& assetRegistry = Grindstone::Editor::Manager::GetInstance().GetAssetRegistry();
	
	size_t stringBlockLength = 0;

	std::string subassetName = std::string("rig-") + orderedSkinnedBones[0].node->mName.C_Str();
	Grindstone::Uuid outUuid = metaFile.GetOrCreateSubassetUuid(subassetName, Grindstone::AssetType::Rig);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / outUuid.ToString();

	std::vector<Grindstone::Formats::Rig::V1::Bone> bones;
	std::vector<char> stringBlockBuffer;

	for (const BoneInfo& boneInfo : orderedSkinnedBones) {
		size_t stringLength = strlen(boneInfo.node->mName.C_Str());
		stringBlockLength += stringLength + 1;
	}

	bones.reserve(orderedSkinnedBones.size());
	stringBlockBuffer.resize(stringBlockLength);

	size_t stringBlockBufferOffset = 0;
	for (const BoneInfo& boneInfo : orderedSkinnedBones) {
		Grindstone::Formats::Rig::V1::Bone& dstBone = bones.emplace_back();
		dstBone.boneNameStringOffset = static_cast<uint32_t>(stringBlockBufferOffset);
		dstBone.boneParentIndex = boneInfo.parentIndex;
		dstBone.inverseBindMatrix = boneInfo.inverseModelMatrix;
		dstBone.localMatrix = boneInfo.offsetMatrix;
		const char* boneName = boneInfo.node->mName.C_Str();

		size_t stringLengthWithNull = strlen(boneName) + 1;
		errno_t cpyRes = strcpy_s(stringBlockBuffer.data() + stringBlockBufferOffset, stringLengthWithNull, boneName);
		GS_ASSERT_ENGINE(cpyRes == 0);

		stringBlockBufferOffset += stringLengthWithNull;
	}

	WriteRig(outputPath, bones, stringBlockBuffer);
}

static void WalkBoneTree(
	uint16_t parentIndex,
	aiNode* node,
	const std::map<std::string, BoneInfo>& skinnedBones,
	std::map<std::string, uint16_t>& nameToBoneIndexMap,
	std::vector<BoneInfo>& sortedNodes
) {
	const std::string name = node->mName.C_Str();
	const auto srcBoneIt = skinnedBones.find(name);
	uint16_t boneIndex = static_cast<uint16_t>(sortedNodes.size());

	if (srcBoneIt != skinnedBones.end()) {
		const BoneInfo& srcBoneInfo = srcBoneIt->second;
		nameToBoneIndexMap[name] = boneIndex;
		BoneInfo& dstBoneInfo = sortedNodes.emplace_back(srcBoneInfo);
		dstBoneInfo.boneIndex = boneIndex;
		dstBoneInfo.parentIndex = static_cast<uint16_t>(parentIndex);
	}
	else {
		BoneInfo& dstBoneInfo = sortedNodes.emplace_back();
		dstBoneInfo.boneIndex = boneIndex;
		dstBoneInfo.parentIndex = static_cast<uint16_t>(parentIndex);
		dstBoneInfo.node = node;
		dstBoneInfo.offsetMatrix = AiMatToGlm(node->mTransformation);
		dstBoneInfo.inverseModelMatrix = AiMatToGlm(node->mTransformation.Inverse());
	}

	for (unsigned int childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		aiNode* childNode = node->mChildren[childIndex];
		WalkBoneTree(boneIndex, childNode, skinnedBones, nameToBoneIndexMap, sortedNodes);
	}
}

// This function finds the parent bone for each bone in the skinnedBones map
// and sets the parentIndex accordingly. It also returns the name of the root node.
static std::pair<std::map<std::string, uint16_t>, std::vector<BoneInfo>> SortBones(std::map<std::string, BoneInfo>& skinnedBones) {
	// Get root node
	aiNode* skeletonRoot = nullptr;
	for (auto& pair : skinnedBones) {
		BoneInfo& boneInfo = pair.second;
		aiNode* lastBoneNode = boneInfo.node;
		aiNode* currentNode = boneInfo.node->mParent;
		while ((currentNode != nullptr)) {
			if (skinnedBones.count(currentNode->mName.data) > 0) {
				lastBoneNode = currentNode;
			}

			currentNode = currentNode->mParent;
		}

		if (lastBoneNode != nullptr) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(skeletonRoot == lastBoneNode || skeletonRoot == nullptr, "There should be exactly one root bone in the skeleton.");
			skeletonRoot = lastBoneNode;
		}
	}

	std::vector<BoneInfo> sortedBones;
	std::map<std::string, uint16_t> nameToBoneIndexMap;
	WalkBoneTree(std::numeric_limits<uint16_t>::max(), skeletonRoot, skinnedBones, nameToBoneIndexMap, sortedBones);

	return { nameToBoneIndexMap, sortedBones };
}

static void JoinMeshBones(const aiMesh* mesh, std::map<std::string, BoneInfo>& skinnedBones) {
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
		aiBone* bone = mesh->mBones[boneIndex];
		const char* name = bone->mName.data;

		if (skinnedBones.find(name) == skinnedBones.end()) {
			uint16_t boneIndex = static_cast<uint16_t>(skinnedBones.size());

			skinnedBones[name] = BoneInfo{
				bone->mNode,
				boneIndex,
				0,
				AiMatToGlm(bone->mOffsetMatrix),
				glm::mat4(1.0f)
			};
		}
		else {
			GS_ASSERT(skinnedBones[name].node == bone->mNode);
		}
	}
}

void ModelImporter::ProcessMaterial(size_t materialIndex, aiMaterial* inputMaterial) {
	aiString Path;
	StandardMaterialCreateInfo newMaterial;

	aiString name;
	inputMaterial->Get(AI_MATKEY_NAME, name);

	if (name.length == 0) {
		name = "Material_" + std::to_string(materialIndex);
	}

	newMaterial.albedoPath = GetTexturePath(baseFolderPath, inputMaterial, aiTextureType_DIFFUSE);
	newMaterial.normalPath = GetTexturePath(baseFolderPath, inputMaterial, aiTextureType_NORMALS);
	newMaterial.specularPath = GetTexturePath(baseFolderPath, inputMaterial, aiTextureType_METALNESS);
	newMaterial.roughnessPath = GetTexturePath(baseFolderPath, inputMaterial, aiTextureType_SHININESS);

	aiColor4D diffuse_color;
	if (AI_SUCCESS == aiGetMaterialColor(inputMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color)) {
		memcpy(&newMaterial.albedoColor, &diffuse_color, sizeof(float) * 4);
	}

	aiColor4D metalness;
	if (AI_SUCCESS == aiGetMaterialColor(inputMaterial, AI_MATKEY_COLOR_SPECULAR, &metalness)) {
		newMaterial.metalness = metalness.r;
	}

	aiColor4D roughness;
	if (AI_SUCCESS == aiGetMaterialColor(inputMaterial, AI_MATKEY_COLOR_AMBIENT, &roughness)) {
		newMaterial.roughness = roughness.r;
	}

	newMaterial.materialName = name.C_Str();
	Grindstone::Uuid uuid = metaFile.GetOrCreateSubassetUuid(newMaterial.materialName, Grindstone::AssetType::Material);
	outputMaterialUuids[materialIndex] = uuid.ToString();

	std::filesystem::path outputPath = assetRegistry->GetCompiledAssetsPath() / outputMaterialUuids[materialIndex];
	CreateStandardMaterial(*assetRegistry, newMaterial, outputPath);
}

void ModelImporter::InitSubmeshes(aiMesh* inputMesh, OutputMesh& outputMesh, bool hasBones) {
	uint32_t& vertexCount = outputMesh.vertexCount;
	uint32_t indexCount = inputMesh->mNumFaces * 3;

	outputMesh.indices.reserve(indexCount);
	outputMesh.submeshes.resize(1);
	Submesh& outputSubmesh = outputMesh.submeshes[0];

	outputSubmesh.baseVertex = 0;
	outputSubmesh.baseIndex = 0;
	outputSubmesh.indexCount = indexCount;
	outputSubmesh.materialIndex = inputMesh->mMaterialIndex;
	outputMesh.vertexCount = inputMesh->mNumVertices;

	size_t vertexCountSizeT = vertexCount;
	outputMesh.vertexArray.position.reserve(vertexCountSizeT * 3);
	outputMesh.vertexArray.normal.reserve(vertexCountSizeT * 3);
	outputMesh.vertexArray.tangent.reserve(vertexCountSizeT * 3);
	outputMesh.vertexArray.texCoordArray.resize(1);
	outputMesh.vertexArray.texCoordArray[0].reserve(vertexCountSizeT * 2);
	size_t boneWeightCount = hasBones
		? vertexCountSizeT * NUM_BONES_PER_VERTEX
		: 0;
	outputMesh.vertexArray.boneIds.resize(boneWeightCount);
	outputMesh.vertexArray.boneWeights.resize(boneWeightCount);
}

void ModelImporter::ProcessVertices(aiMesh* inputMesh, OutputMesh& outputMesh) {
	const aiVector3D zeroVector(0.0f, 0.0f, 0.0f);

	for (unsigned int vertexIterator = 0; vertexIterator < inputMesh->mNumVertices; vertexIterator++) {
		const aiVector3D* aiPos = &(inputMesh->mVertices[vertexIterator]);
		const aiVector3D* aiNormal = &(inputMesh->mNormals[vertexIterator]);
		const aiVector3D* aiTangent = &(inputMesh->mTangents[vertexIterator]);
		const aiVector3D* aiTexCoord = inputMesh->HasTextureCoords(0)
			? &(inputMesh->mTextureCoords[0][vertexIterator])
			: &zeroVector;

		OutputMesh::VertexArray& vertexArray = outputMesh.vertexArray;
		PushVertex3dToVector(vertexArray.position, aiPos);
		PushVertex3dToVector(vertexArray.normal, aiNormal);
		PushVertex3dToVector(vertexArray.tangent, aiTangent);
		PushVertex2dToVector(vertexArray.texCoordArray[0], aiTexCoord);
	}

	aiVector3D& minAABB = inputMesh->mAABB.mMin;
	aiVector3D& maxAABB = inputMesh->mAABB.mMax;
	outputMesh.boundingData.minAABB = glm::vec3(minAABB.x, minAABB.y, minAABB.z);
	outputMesh.boundingData.maxAABB = glm::vec3(maxAABB.x, maxAABB.y, maxAABB.z);
	outputMesh.boundingData.sphereCenter = (outputMesh.boundingData.maxAABB + outputMesh.boundingData.minAABB) / 2.0f;
	outputMesh.boundingData.sphereRadius = glm::distance(outputMesh.boundingData.maxAABB, outputMesh.boundingData.sphereCenter);

	for (unsigned int faceIterator = 0; faceIterator < inputMesh->mNumFaces; faceIterator++) {
		const aiFace& face = inputMesh->mFaces[faceIterator];
		outputMesh.indices.push_back(face.mIndices[0]);
		outputMesh.indices.push_back(face.mIndices[1]);
		outputMesh.indices.push_back(face.mIndices[2]);
	}
}

void ModelImporter::ProcessNodeTree(aiNode* inputNode, size_t parentIndex) {
	size_t nodeIndex = outputNodes.size();

	OutputNode& node = outputNodes.emplace_back();
	if (inputNode->mName.length == 0) {
		node.name = std::string("Unnamed entity ") + std::to_string(nodeIndex);
	}
	else {
		node.name = std::string(inputNode->mName.C_Str());
	}

	inputNode->mTransformation.Decompose(node.scale, node.rotation, node.position);
	node.parentNode = parentIndex;
	if (inputNode->mNumMeshes > 0) {
		node.meshIndex = inputNode->mMeshes[0];
	}

	for (unsigned int meshIndex = 1; meshIndex < inputNode->mNumMeshes; meshIndex++) {
		OutputNode& meshNode = outputNodes.emplace_back();
		meshNode.name = outputNodes[nodeIndex].name + " - Mesh " + std::to_string(meshIndex);
		meshNode.parentNode = nodeIndex;
		meshNode.meshIndex = inputNode->mMeshes[meshIndex];
	}

	for (unsigned int childIterator = 0; childIterator < inputNode->mNumChildren; ++childIterator) {
		ProcessNodeTree(inputNode->mChildren[childIterator], nodeIndex);
	}
}

void ModelImporter::ProcessVertexBoneWeights(const aiMesh* inputMesh, const std::map<std::string, uint16_t>& nameToBoneIndexMap, OutputMesh& outputMesh) {
	size_t vertCount = outputMesh.vertexArray.position.size() * NUM_BONES_PER_VERTEX;
	outputMesh.vertexArray.boneWeights.resize(vertCount);
	outputMesh.vertexArray.boneIds.resize(vertCount);

	for (unsigned int boneIterator = 0; boneIterator < inputMesh->mNumBones; boneIterator++) {
		aiBone* bone = inputMesh->mBones[boneIterator];
		std::string boneName(bone->mName.data);
		auto boneIt = nameToBoneIndexMap.find(boneName);
		GS_ASSERT_ENGINE(boneIt != nameToBoneIndexMap.end());

		for (unsigned int weightIterator = 0; weightIterator < bone->mNumWeights; weightIterator++) {
			aiVertexWeight& weight = bone->mWeights[weightIterator];

			unsigned int vertexId = weight.mVertexId;
			float vertexWeight = weight.mWeight;
			AddBoneData(outputMesh, vertexId, boneIt->second, vertexWeight);
		}
	}

	if (hasExtraWeights) {
		NormalizeBoneWeights(outputMesh);
	}
}

void ModelImporter::NormalizeBoneWeights(OutputMesh& outputMesh) {
	for (size_t i = 0; i < outputMesh.vertexArray.boneWeights.size(); i += NUM_BONES_PER_VERTEX) {
		float w0 = outputMesh.vertexArray.boneWeights[i];
		float w1 = outputMesh.vertexArray.boneWeights[i + 1];
		float w2 = outputMesh.vertexArray.boneWeights[i + 2];
		float w3 = outputMesh.vertexArray.boneWeights[i + 3];
		float total = w0 + w1 + w2 + w3;

		outputMesh.vertexArray.boneWeights[i] = w0 / total;
		outputMesh.vertexArray.boneWeights[i + 1] = w1 / total;
		outputMesh.vertexArray.boneWeights[i + 2] = w2 / total;
		outputMesh.vertexArray.boneWeights[i + 3] = w3 / total;
	}
}

void ModelImporter::AddBoneData(OutputMesh& outputMesh, unsigned int vertexId, unsigned int boneId, float vertexWeight) {
	unsigned int baseIndex = vertexId * NUM_BONES_PER_VERTEX;
	unsigned int lastIndex = baseIndex + NUM_BONES_PER_VERTEX;
	for (unsigned int i = baseIndex; i < lastIndex; i++) {
		if (outputMesh.vertexArray.boneWeights[i] == 0.0f) {
			outputMesh.vertexArray.boneIds[i] = boneId;
			outputMesh.vertexArray.boneWeights[i] = vertexWeight;
			return;
		}
	}

	// Too many boneweights - replace the smallest one
	hasExtraWeights = true;
	unsigned int lowestIndex = baseIndex;
	float lowestWeight = outputMesh.vertexArray.boneWeights[lowestIndex];
	for (unsigned int i = baseIndex; i < lastIndex; i++) {
		float currentWeight = outputMesh.vertexArray.boneWeights[i];
		if (currentWeight < lowestWeight) {
			lowestIndex = i;
			lowestWeight = currentWeight;
		}
	}

	if (lowestWeight < vertexWeight) {
		outputMesh.vertexArray.boneIds[lowestIndex] = boneId;
		outputMesh.vertexArray.boneWeights[lowestIndex] = vertexWeight;
	}
}

void ModelImporter::ProcessLight(aiLight* light) {
	aiString lightName = light->mName;
	for (OutputNode& node : outputNodes) {
		if (node.name == lightName.C_Str()) {
			node.lightData = light;
		}
	}
}

void ModelImporter::ProcessCamera(aiCamera* camera) {
	aiString cameraName = camera->mName;
	for (OutputNode& node : outputNodes) {
		if (node.name == cameraName.C_Str()) {
			node.cameraData = camera;
		}
	}
}

void ModelImporter::ProcessAnimation(aiAnimation* animation, const std::map<std::string, uint16_t>& nameToBoneIndexMap) {
	std::string animationName(animation->mName.data);

	double ticksPerSecond = animation->mTicksPerSecond != 0
		? animation->mTicksPerSecond
		: 25.0f;
	double duration = animation->mDuration;

	std::string subassetName = "anim-" + animationName;
	Grindstone::Uuid outUuid = metaFile.GetOrCreateSubassetUuid(subassetName, Grindstone::AssetType::AnimationClip);

	std::filesystem::path outputPath = assetRegistry->GetCompiledAssetsPath() / outUuid.ToString();
	std::ofstream output(outputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
	}

	std::vector<Grindstone::Formats::Animation::V1::BoneChannel> channels;
	channels.resize(animation->mNumChannels);
	Grindstone::Formats::Animation::V1::BoneChannelData dstChannelData;
	std::vector<char> stringBlockBuffer;

	size_t positionKeyframeCount = 0;
	size_t rotationKeyframeCount = 0;
	size_t scaleKeyframeCount = 0;
	size_t stringBlockBufferSize = 0;

	// Reserve bone data.
	for (unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
		aiNodeAnim* srcChannel = animation->mChannels[channelIndex];
		std::string channelName(srcChannel->mNodeName.data);
		auto boneIterator = nameToBoneIndexMap.find(channelName);
		bool isValidBone = boneIterator != nameToBoneIndexMap.end();

		if (isValidBone) {
			positionKeyframeCount += static_cast<size_t>(srcChannel->mNumPositionKeys);
			rotationKeyframeCount += static_cast<size_t>(srcChannel->mNumRotationKeys);
			scaleKeyframeCount += static_cast<size_t>(srcChannel->mNumScalingKeys);
			stringBlockBufferSize += static_cast<size_t>(srcChannel->mNodeName.length) + 1;
		}
	}

	dstChannelData.positions.reserve(positionKeyframeCount);
	dstChannelData.rotations.reserve(rotationKeyframeCount);
	dstChannelData.scales.reserve(scaleKeyframeCount);
	stringBlockBuffer.resize(stringBlockBufferSize);

	// Extract bone channel data.
	positionKeyframeCount = 0;
	rotationKeyframeCount = 0;
	scaleKeyframeCount = 0;
	stringBlockBufferSize = 0;
	for (unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
		Grindstone::Formats::Animation::V1::BoneChannel& dstChannel = channels[channelIndex];
		aiNodeAnim* srcChannel = animation->mChannels[channelIndex];
		std::string channelName(srcChannel->mNodeName.data);

		errno_t cpyRes = strcpy_s(stringBlockBuffer.data() + stringBlockBufferSize, channelName.size() + 1, channelName.data());
		GS_ASSERT(cpyRes == 0)

		dstChannel.boneNameStringOffset = static_cast<uint32_t>(stringBlockBufferSize);
		stringBlockBufferSize += channelName.size() + 1;

		dstChannel.positionKeyOffset = positionKeyframeCount;
		dstChannel.rotationKeyOffset = rotationKeyframeCount;
		dstChannel.scaleKeyOffset = scaleKeyframeCount;

		dstChannel.positionCount = static_cast<uint16_t>(srcChannel->mNumPositionKeys);
		dstChannel.rotationCount = static_cast<uint16_t>(srcChannel->mNumRotationKeys);
		dstChannel.scaleCount = static_cast<uint16_t>(srcChannel->mNumScalingKeys);

		positionKeyframeCount += static_cast<size_t>(srcChannel->mNumPositionKeys);
		rotationKeyframeCount += static_cast<size_t>(srcChannel->mNumRotationKeys);
		scaleKeyframeCount += static_cast<size_t>(srcChannel->mNumScalingKeys);

		for (unsigned int i = 0; i < srcChannel->mNumPositionKeys; ++i) {
			double time = srcChannel->mPositionKeys[i].mTime;
			aiVector3D& srcValue = srcChannel->mPositionKeys[i].mValue;
			Grindstone::Math::Float3 value = Grindstone::Math::Float3(srcValue.x, srcValue.y, srcValue.z);
			dstChannelData.positions.emplace_back(time, value);
		}

		for (unsigned int i = 0; i < srcChannel->mNumRotationKeys; ++i) {
			double time = srcChannel->mRotationKeys[i].mTime;
			aiQuaternion& srcValue = srcChannel->mRotationKeys[i].mValue;
			Grindstone::Math::Quaternion value = Grindstone::Math::Quaternion(srcValue.x, srcValue.y, srcValue.z, srcValue.w);
			dstChannelData.rotations.emplace_back(time, value);
		}

		for (unsigned int i = 0; i < srcChannel->mNumScalingKeys; ++i) {
			double time = srcChannel->mScalingKeys[i].mTime;
			aiVector3D& srcValue = srcChannel->mScalingKeys[i].mValue;
			Grindstone::Math::Float3 value = Grindstone::Math::Float3(srcValue.x, srcValue.y, srcValue.z);
			dstChannelData.scales.emplace_back(time, value);
		}
	}

	const size_t headerSize = sizeof(Grindstone::Formats::Animation::V1::Header);
	const size_t boneChannelsSize = GetVectorSize(channels);
	const size_t positionSize = GetVectorSize(dstChannelData.positions);
	const size_t rotationSize = GetVectorSize(dstChannelData.rotations);
	const size_t scaleSize = GetVectorSize(dstChannelData.scales);

	Grindstone::Formats::Animation::V1::Header header{
		.version = Grindstone::Formats::Animation::V1::version,
		.animationDuration = duration,
		.ticksPerSecond = ticksPerSecond,
		.boneChannelCount = static_cast<uint16_t>(animation->mNumChannels),
		.propertyChannelCount = 0,
		.positionKeyframesCount = static_cast<uint32_t>(dstChannelData.positions.size()),
		.rotationKeyframesCount = static_cast<uint32_t>(dstChannelData.positions.size()),
		.scaleKeyframesCount = static_cast<uint32_t>(dstChannelData.positions.size()),
		.eventCount = 0,
	};

	header.boneChannelDataOffset = Grindstone::Formats::Animation::V1::magicSize + headerSize;
	header.propertyChannelDataOffset = header.boneChannelDataOffset + boneChannelsSize;
	header.positionKeyframesOffset = header.propertyChannelDataOffset + 0;
	header.rotationKeyframesOffset = header.positionKeyframesOffset + positionSize;
	header.scaleKeyframesOffset = header.rotationKeyframesOffset + rotationSize;
	header.propertyKeyframesOffset = header.scaleKeyframesOffset + scaleSize;
	header.eventsArrayOffset = header.propertyKeyframesOffset + 0;
	header.eventsPayloadOffset = header.eventsArrayOffset + 0;
	header.stringBlockOffset = header.eventsPayloadOffset + 0;
	header.totalFileSize = header.stringBlockOffset + stringBlockBuffer.size();

	//  - Output File MetaData
	output.write(Grindstone::Formats::Animation::V1::magicCode, Grindstone::Formats::Animation::V1::magicSize);
	output.write(reinterpret_cast<const char*>(&header), headerSize);
	OutputVector(output, channels, boneChannelsSize);
	OutputVector(output, dstChannelData.positions, positionSize);
	OutputVector(output, dstChannelData.rotations, rotationSize);
	OutputVector(output, dstChannelData.scales, scaleSize);
	output.write(stringBlockBuffer.data(), stringBlockBuffer.size());
}

void ModelImporter::Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) {
	this->path = path;
	this->baseFolderPath = this->path.parent_path();
	this->assetManager = &assetManager;
	this->assetRegistry = &assetRegistry;

	Assimp::Importer importer;
	int importFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_Triangulate |
		aiProcess_GenBoundingBoxes;

	metaFile = assetRegistry.GetMetaFileByPath(path);

	Grindstone::Editor::ImporterSettings& settings = metaFile.GetImporterSettings();
	bool shouldImportScene = settings.Get("ImportScene", true);
	bool shouldImportLights = settings.Get("ImportLights", true);
	bool shouldImportCameras = settings.Get("ImportCameras", true);
	bool shouldImportAnimations = settings.Get("ImportAnims", true);
	bool shouldImportRig = settings.Get("ImportRigs", true);

	if (settings.Get("FlipUVs", true)) {
		importFlags |= aiProcess_FlipUVs;
	}

	if (settings.Get("ReduceDuplicateMeshes", true)) {
		importFlags |= aiProcess_FindInstances;
	}

	if (settings.Get("FlipFaces", false)) {
		importFlags |= aiProcess_FlipWindingOrder;
	}

	if (settings.Get("OptimizeMeshes", true)) {
		importFlags |= aiProcess_OptimizeMeshes;
	}

	if (settings.Get("OptimizeScene", true)) {
		importFlags |= aiProcess_OptimizeGraph;
	}

	if (settings.Get("SplitLargeMeshes", false)) {
		importFlags |= aiProcess_SplitLargeMeshes;
	}

	if (settings.Get("IsLeftHanded", false)) {
		importFlags |= aiProcess_MakeLeftHanded;
	}

	double scale = settings.Get("Scale", 1.0);
	if (scale <= 0.0) {
		scale = 1.0;
	}

	importFlags |= aiProcess_PopulateArmatureData;
	importFlags |= aiProcess_GlobalScale;
	importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, static_cast<ai_real>(scale));

	scene = importer.ReadFile(
		path.string(),
		importFlags
	);

	if (!scene) {
		GPRINT_ERROR_V(Grindstone::LogSource::EditorImporter, "Model Importer: Unable to load scene from '{}'.", importer.GetErrorString());
		return;
	}

	glm::mat4 globalInverseTransform = AiMatToGlm(scene->mRootNode->mTransformation.Inverse());

	isSkeletalMesh = scene->hasSkeletons();

	outputMaterialUuids.resize(scene->mNumMaterials);
	for (unsigned int materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		ProcessMaterial(materialIndex, material);
	}

	if (shouldImportRig) {
	}

	std::map<std::string, uint16_t> nameToBoneIndexMap;
	std::vector<BoneInfo> orderedSkinnedBones;
	if (shouldImportRig) {
		for (unsigned int skeletonIndex = 0; skeletonIndex < scene->mNumSkeletons; ++skeletonIndex) {
			aiSkeleton* skeleton = scene->mSkeletons[skeletonIndex];
			ProcessSkeletonRig(metaFile, skeleton);
		}

		std::map<std::string, BoneInfo> skinnedBones;
		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			aiMesh* inputMesh = scene->mMeshes[meshIndex];
			JoinMeshBones(inputMesh, skinnedBones);
		}

		auto [nameToBoneIndexMapTmp, orderedSkinnedBonesTmp] = SortBones(skinnedBones);
		nameToBoneIndexMap = nameToBoneIndexMapTmp;
		orderedSkinnedBones = orderedSkinnedBonesTmp;
		ProcessMeshRig(metaFile, nameToBoneIndexMap, orderedSkinnedBones);
	}

	std::vector<glm::mat4> boneMatrices;
	ProcessNodeTree(scene->mRootNode, SIZE_MAX);
	outputMeshUuids.resize(scene->mNumMeshes);
	outputMeshes.resize(scene->mNumMeshes);
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* inputMesh = scene->mMeshes[meshIndex];
		OutputMesh& outputMesh = outputMeshes[meshIndex];
		outputMesh.name = inputMesh->mName.length == 0
			? std::string("Mesh ") + std::to_string(meshIndex)
			: inputMesh->mName.C_Str();

		for (unsigned int tempMeshIndex = 0; tempMeshIndex < meshIndex; ++tempMeshIndex) {
			if (outputMesh.name == outputMeshes[tempMeshIndex].name) {
				outputMesh.name = outputMesh.name + std::to_string(meshIndex);
			}
		}

		InitSubmeshes(inputMesh, outputMesh, isSkeletalMesh);
		ProcessVertices(inputMesh, outputMesh);
		ProcessVertexBoneWeights(inputMesh, nameToBoneIndexMap, outputMesh);
	}

	if (shouldImportLights) {
		for (unsigned int lightIndex = 0; lightIndex < scene->mNumLights; ++lightIndex) {
			aiLight* light = scene->mLights[lightIndex];
			ProcessLight(light);
		}
	}

	if (shouldImportCameras) {
		for (unsigned int cameraIndex = 0; cameraIndex < scene->mNumCameras; ++cameraIndex) {
			aiCamera* camera = scene->mCameras[cameraIndex];
			ProcessCamera(camera);
		}
	}

	if (shouldImportAnimations) {
		for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
			aiAnimation* animation = scene->mAnimations[i];
			ProcessAnimation(animation, nameToBoneIndexMap);
		}
	}

	importer.FreeScene();

	for (size_t meshIndex = 0; meshIndex < outputMeshes.size(); meshIndex++) {
		const OutputMesh& mesh = outputMeshes[meshIndex];
		WriteMesh(meshIndex, mesh);
	}

	if (shouldImportScene) {
		WritePrefab();
	}

	metaFile.Save(modelImporterVersion);
}

inline static void WriteComponentHeader(std::ofstream& output, const char* name) {
	output << "\t\t\t\t{\n\t\t\t\t\t\"component\": \"" << name << "\",\n\t\t\t\t\t\"params\": {\n";
}

inline static std::ofstream& WriteComponentKey(std::ofstream& output, const char* key) {
	output << "\t\t\t\t\t\t\"" << key << "\": ";
	return output;
}

inline static void WriteComponentFooter(std::ofstream& output, bool isBeforeLast) {
	output << (isBeforeLast
		? "\t\t\t\t\t}\n\t\t\t\t},\n"
		: "\t\t\t\t\t}\n\t\t\t\t}\n");
}

static std::ostream& operator<<(std::ostream& output, const aiVector3D& vector) {
	output << "[" << vector.x << ", " << vector.y << ", " << vector.z << "]";
	return output;
}

static std::ostream& operator<<(std::ostream& output, const aiQuaternion& quaternion) {
	output << "[" << quaternion.x << ", " << quaternion.y << ", " << quaternion.z << ", " << quaternion.w << "]";
	return output;
}

static std::ostream& operator<<(std::ostream& output, const aiColor3D& vector) {
	output << "[" << vector.r << ", " << vector.g << ", " << vector.b << "]";
	return output;
}

void ModelImporter::WritePrefab() {
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid outUuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, Grindstone::AssetType::Scene);

	std::filesystem::path meshOutputPath = assetRegistry->GetCompiledAssetsPath() / outUuid.ToString();
	std::ofstream output(meshOutputPath, std::ios::binary);

	output << "{\n\t\"name\": \"" << subassetName << "\",\n\t\"entities\": [\n";
	for (size_t nodeIndex = 0; nodeIndex < outputNodes.size(); ++nodeIndex) {
		OutputNode& node = outputNodes[nodeIndex];

		bool hasParent = node.parentNode != SIZE_MAX;
		bool hasMesh = node.meshIndex != SIZE_MAX;
		bool hasLight = node.lightData != nullptr &&
			(
				(node.lightData->mType == aiLightSource_DIRECTIONAL) ||
				(node.lightData->mType == aiLightSource_POINT) ||
				(node.lightData->mType == aiLightSource_SPOT)
			);
		bool hasCamera = node.cameraData != nullptr;

		output << "\t\t{\n\t\t\t\"entityId\": " << nodeIndex << ",\n\t\t\t\"components\": [\n";

		// Output Tag
		WriteComponentHeader(output, "Tag");
		WriteComponentKey(output, "tag") << "\"" << node.name << "\"\n";
		WriteComponentFooter(output, true);

		// Output Transform
		WriteComponentHeader(output, "Transform");
		WriteComponentKey(output, "position") << node.position << ",\n";
		WriteComponentKey(output, "rotation") << node.rotation << ",\n";
		WriteComponentKey(output, "scale") << node.scale << "\n";
		WriteComponentFooter(output, hasParent || hasMesh || hasCamera || hasLight);

		// Output Parent
		if (hasParent) {
			WriteComponentHeader(output, "Parent");
			WriteComponentKey(output, "parentEntity") << node.parentNode << "\n";
			WriteComponentFooter(output, hasMesh || hasCamera || hasLight);
		}

		// Output Mesh and MeshRenderer
		if (hasMesh) {
			OutputMesh& mesh = outputMeshes[node.meshIndex];
			if (mesh.submeshes.size() == 0) {
				GS_ASSERT_LOG("No submeshes found in asset.");
			}
			else if (mesh.submeshes.size() > 1) {
				GS_ASSERT_LOG("More than one submesh found in asset.");
			}
			else {
				std::string& meshUuid = outputMeshUuids[node.meshIndex];
				std::string& materialUuid = outputMaterialUuids[mesh.submeshes[0].materialIndex];
				WriteComponentHeader(output, "MeshRenderer");
				WriteComponentKey(output, "materials") << "[ \"" << materialUuid << "\" ]\n";
				WriteComponentFooter(output, true);
				WriteComponentHeader(output, "Mesh");
				WriteComponentKey(output, "mesh") << "\"" << meshUuid << "\"\n";
				WriteComponentFooter(output, hasCamera || hasLight);
			}
		}

		// Output Camera
		if (hasCamera) {
			WriteComponentHeader(output, "Camera");
			WriteComponentKey(output, "isMainCamera") << "false,\n";
			WriteComponentKey(output, "nearPlaneDistance") << node.cameraData->mClipPlaneNear << ",\n";
			WriteComponentKey(output, "farPlaneDistance") << node.cameraData->mClipPlaneFar << ",\n";
			WriteComponentKey(output, "fieldOfView") << node.cameraData->mHorizontalFOV << ",\n";
			WriteComponentKey(output, "aspectRatio") << node.cameraData->mAspect << "\n";
			// TODO: Handle Orthographic/Perspective, Orthographic Width, and LookAt
			WriteComponentFooter(output, hasLight);
		}

		// Output Light
		if (hasLight) {
			// TODO: Use node.lightData->mDirection;
			switch (node.lightData->mType) {
			case aiLightSource_DIRECTIONAL:
				WriteComponentHeader(output, "DirectionalLight");
				WriteComponentKey(output, "color") << node.lightData->mColorDiffuse << ",\n";
				WriteComponentKey(output, "sourceRadius") << "40.0\n";
				WriteComponentKey(output, "intensity") << "1.0,\n";
				WriteComponentKey(output, "shadowResolution") << "2048.0\n";
				WriteComponentFooter(output, false);
				break;
			case aiLightSource_POINT:
				WriteComponentHeader(output, "PointLight");
				WriteComponentKey(output, "color") << node.lightData->mColorDiffuse << ",\n";
				WriteComponentKey(output, "attenuationRadius") << node.lightData->mAttenuationQuadratic << ",\n";
				WriteComponentKey(output, "intensity") << "1.0\n";
				WriteComponentFooter(output, false);
				break;
			case aiLightSource_SPOT:
				WriteComponentHeader(output, "SpotLight");
				WriteComponentKey(output, "color") << node.lightData->mColorDiffuse << ",\n";
				WriteComponentKey(output, "attenuationRadius") << node.lightData->mAttenuationQuadratic << ",\n";
				WriteComponentKey(output, "intensity") << "1.0,\n";
				WriteComponentKey(output, "innerAngle") << node.lightData->mAngleInnerCone << ",\n";
				WriteComponentKey(output, "outerAngle") << node.lightData->mAngleOuterCone << ",\n";
				WriteComponentKey(output, "shadowResolution") << "0.0\n";
				WriteComponentFooter(output, false);
				break;
			default:
				GS_ASSERT_LOG("Unexpected light type.");
			}
		}

		output << "\t\t\t]\n";

		if (nodeIndex == outputNodes.size() - 1) {
			output << "\t\t}\n";
		}
		else {
			output << "\t\t},\n";
		}
	}

	output << "\t]\n}\n";

}

void ModelImporter::WriteMesh(size_t index, const OutputMesh& mesh) {
	const std::string& subassetName = mesh.name;

	Grindstone::Uuid outUuid = metaFile.GetOrCreateSubassetUuid(subassetName, Grindstone::AssetType::Mesh3d);
	outputMeshUuids[index] = outUuid.ToString();

	std::filesystem::path meshOutputPath = assetRegistry->GetCompiledAssetsPath() / outUuid.ToString();

	auto meshCount = mesh.submeshes.size();

	Grindstone::Formats::Model::V1::Header outHeader;
	outHeader.totalFileSize = static_cast<uint64_t>(
		3 +
		sizeof(outHeader) +
		sizeof(Grindstone::Formats::Model::V1::BoundingData) +
		meshCount * sizeof(Submesh) +
		mesh.vertexArray.position.size() * sizeof(float) +
		mesh.vertexArray.normal.size() * sizeof(float) +
		mesh.vertexArray.tangent.size() * sizeof(float) +
		mesh.vertexArray.texCoordArray[0].size() * sizeof(float) +
		mesh.indices.size() * sizeof(uint16_t)
	);

	outHeader.hasVertexPositions = true;
	outHeader.hasVertexNormals = true;
	outHeader.hasVertexTangents = true;
	outHeader.vertexUvSetCount = 1;
	outHeader.numWeightPerBone = isSkeletalMesh ? 4 : 0;
	outHeader.vertexCount = static_cast<uint64_t>(mesh.vertexCount);
	outHeader.indexCount = static_cast<uint64_t>(mesh.indices.size());
	outHeader.meshCount = static_cast<uint32_t>(meshCount);

	if (isSkeletalMesh) {
		outHeader.totalFileSize += static_cast<uint32_t>(
			mesh.vertexArray.boneIds.size() * sizeof(uint16_t) +
			mesh.vertexArray.boneWeights.size() * sizeof(float)
		);
	}

	std::ofstream output(meshOutputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + meshOutputPath.string());
	}

	//  - Output File MetaData
	output.write("GMF", 3);
	output.write(reinterpret_cast<const char*>(&outHeader), sizeof(Grindstone::Formats::Model::V1::Header));
	output.write(reinterpret_cast<const char*>(&mesh.boundingData), sizeof(Grindstone::Formats::Model::V1::BoundingData));
	for (uint32_t i = 0; i < mesh.submeshes.size(); ++i) {
		Submesh normalizedSubmesh = mesh.submeshes[i];
		normalizedSubmesh.materialIndex = i;
		output.write(reinterpret_cast<const char*>(&normalizedSubmesh), sizeof(Submesh));
	}

	OutputVertexArray(output, mesh.vertexArray.position);
	OutputVertexArray(output, mesh.vertexArray.normal);
	OutputVertexArray(output, mesh.vertexArray.tangent);
	OutputVertexArray(output, mesh.vertexArray.texCoordArray[0]);

	if (isSkeletalMesh) {
		OutputVertexArray(output, mesh.vertexArray.boneIds);
		OutputVertexArray(output, mesh.vertexArray.boneWeights);
	}

	// - Output Indices
	output.write(reinterpret_cast<const char*>(mesh.indices.data()), mesh.indices.size() * sizeof(uint16_t));
	
	output.close();

	assetManager->QueueReloadAsset(Grindstone::AssetType::Mesh3d, outUuid);
}

void Grindstone::Editor::Importers::ImportModel(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) {
	ModelImporter modelImporter;
	modelImporter.Import(assetRegistry, assetManager, path);
}
