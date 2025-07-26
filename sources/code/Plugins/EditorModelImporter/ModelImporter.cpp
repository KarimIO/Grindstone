#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <Common/Formats/Animation.hpp>
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>

#include "ModelImporter.hpp"
#include "ModelMaterialImporter.hpp"

using namespace Grindstone::Editor::Importers;


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
	std::map<std::string, glm::mat4> tempOffsetMatrices; // Save string->offset matrix so we can use it when constructing the bone data
	std::map<std::string, unsigned int> boneMapping;
	const aiScene* scene = nullptr;
	bool hasExtraWeights = false;
	bool isSkeletalMesh = false;

	struct BoneData {
		uint16_t parentIndex;
		glm::mat4 offsetMatrix;
		glm::mat4 inverseModelMatrix;

		BoneData(uint16_t parentIndex, glm::mat4& offsetMatrix, glm::mat4& inverseMatrix) {
			this->parentIndex = parentIndex;
			this->offsetMatrix = offsetMatrix;
			this->inverseModelMatrix = inverseMatrix;
		}
	};

	struct OutputMesh {
		std::string name;
		Grindstone::Formats::Model::V1::BoundingData boundingData;
		uint32_t vertexCount = 0;
		uint16_t boneCount = 0;
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
		std::vector<BoneData> bones;
		std::vector<std::string> boneNames;
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
	void ProcessSkeleton(aiSkeleton* skeleton);
	void ProcessVertexBoneWeights(aiMesh* inputMesh, OutputMesh& outputMesh);
	void NormalizeBoneWeights(OutputMesh& outputMesh);
	void ProcessAnimation(aiAnimation* animation);
	void AddBoneData(OutputMesh& outputMesh, unsigned int vertexId, unsigned int boneId, float vertexWeight);
	void InitSubmeshes(aiMesh* inputMesh, OutputMesh& outputMesh, bool hasBones);
	void ProcessVertices(aiMesh* inputMesh, OutputMesh& outputMesh);
	void WritePrefab();
	void WriteMesh(size_t index, const OutputMesh& mesh);
	void WriteSkeleton(size_t index) const;
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

// Make a list of used bones so we can remove unnecessary bones, and get offset Matrix
void ModelImporter::ProcessSkeleton(aiSkeleton* skeleton) {
	for (unsigned int boneIterator = 0; boneIterator < skeleton->mNumBones; boneIterator++) {
		aiSkeletonBone* bone = skeleton->mBones[boneIterator];
		std::string boneName(bone->mNode->mName.data);

		tempOffsetMatrices[boneName] = AiMatToGlm(bone->mOffsetMatrix);
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

void ModelImporter::ProcessVertexBoneWeights(aiMesh* inputMesh, OutputMesh& outputMesh) {
	for (unsigned int boneIterator = 0; boneIterator < inputMesh->mNumBones; boneIterator++) {
		aiBone* bone = inputMesh->mBones[boneIterator];
		std::string boneName(bone->mName.data);
		unsigned int boneId = boneMapping[boneName];

		for (unsigned int weightIterator = 0; weightIterator < bone->mNumWeights; weightIterator++) {
			auto& weight = bone->mWeights[weightIterator];

			auto vertexId = weight.mVertexId;
			float vertexWeight = weight.mWeight;
			AddBoneData(outputMesh, vertexId, boneId, vertexWeight);
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

void ModelImporter::ProcessAnimation(aiAnimation* animation) {
	std::string animationName(animation->mName.data);

	double ticksPerSecond = animation->mTicksPerSecond != 0
		? animation->mTicksPerSecond
		: 25.0f;
	double duration = animation->mDuration;

	Grindstone::Formats::Animation::V1::Header animationHeader;
	animationHeader.animationDuration = duration;
	animationHeader.channelCount = static_cast<uint16_t>(animation->mNumChannels);
	animationHeader.ticksPerSecond = ticksPerSecond;

	std::vector<Grindstone::Formats::Animation::V1::Channel> channels;
	channels.resize(animation->mNumChannels);
	std::vector<Grindstone::Formats::Animation::V1::ChannelData> channelData;
	channels.resize(animation->mNumChannels);

	std::string subassetName = "anim-" + animationName;
	Grindstone::Uuid outUuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, Grindstone::AssetType::Animation);

	std::filesystem::path outputPath = assetRegistry->GetCompiledAssetsPath() / outUuid.ToString();
	std::ofstream output(outputPath, std::ios::binary);

	if (!output.is_open()) {
		throw std::runtime_error(std::string("Failed to open ") + outputPath.string());
	}

	//  - Output File MetaData
	output.write("GAF", 3);

	for (unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
		Grindstone::Formats::Animation::V1::Channel& dstChannel = channels[channelIndex];
		Grindstone::Formats::Animation::V1::ChannelData& dstChannelData = channelData[channelIndex];
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
				Grindstone::Math::Float3 value = Grindstone::Math::Float3(srcValue.x, srcValue.y, srcValue.z);
				dstChannelData.positions.emplace_back(time, value);
			}

			dstChannelData.rotations.reserve(srcChannel->mNumRotationKeys);
			for (unsigned int i = 0; i < srcChannel->mNumRotationKeys; ++i) {
				double time = srcChannel->mRotationKeys[i].mTime;
				aiQuaternion& srcValue = srcChannel->mRotationKeys[i].mValue;
				Grindstone::Math::Quaternion value = Grindstone::Math::Quaternion(srcValue.x, srcValue.y, srcValue.z, srcValue.w);
				dstChannelData.rotations.emplace_back(time, value);
			}

			dstChannelData.scales.reserve(srcChannel->mNumScalingKeys);
			for (unsigned int i = 0; i < srcChannel->mNumScalingKeys; ++i) {
				double time = srcChannel->mScalingKeys[i].mTime;
				aiVector3D& srcValue = srcChannel->mScalingKeys[i].mValue;
				Grindstone::Math::Float3 value = Grindstone::Math::Float3(srcValue.x, srcValue.y, srcValue.z);
				dstChannelData.scales.emplace_back(time, value);
			}
		}
	}
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

	importFlags |= aiProcess_GlobalScale;
	importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, static_cast<ai_real>(scale));

	scene = importer.ReadFile(
		path.string(),
		importFlags
	);

	if (!scene) {
		throw std::runtime_error(importer.GetErrorString());
	}

	// Set to false, will check if true later.
	bool shouldImportAnimations = false;
	isSkeletalMesh = false; // scene->hasSkeletons();

	outputMaterialUuids.resize(scene->mNumMaterials);
	for (unsigned int materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		ProcessMaterial(materialIndex, material);
	}

	for (unsigned int skeletonIndex = 0; skeletonIndex < scene->mNumSkeletons; ++skeletonIndex) {
		aiSkeleton* skeleton = scene->mSkeletons[skeletonIndex];
		ProcessSkeleton(skeleton);
	}

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
		ProcessVertexBoneWeights(inputMesh, outputMesh);
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
			ProcessAnimation(animation);
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
	outHeader.totalFileSize = static_cast<uint32_t>(
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
	
	// - Output Bone Names
	for (auto& name : mesh.boneNames) {
		output.write(name.data(), name.size() + 1); // size + 1 to include null terminated character
	}

	output.close();

	assetManager->QueueReloadAsset(Grindstone::AssetType::Mesh3d, outUuid);
}

void ModelImporter::WriteSkeleton(size_t index) const {
}

void Grindstone::Editor::Importers::ImportModel(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) {
	ModelImporter modelImporter;
	modelImporter.Import(assetRegistry, assetManager, path);
}
