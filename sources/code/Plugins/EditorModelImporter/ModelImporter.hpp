#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <Common/Formats/Model.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	const uint16_t NUM_BONES_PER_VERTEX = 4;

	class ModelImporter : public Importer {
	public:
		void Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) override;
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

	const Grindstone::Editor::ImporterVersion modelImporterVersion = 1;

	void ImportModel(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path);
}
