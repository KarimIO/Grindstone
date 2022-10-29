#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "Importer.hpp"

namespace Grindstone {
	namespace Importers {
		const uint16_t NUM_BONES_PER_VERTEX = 4;

		class ModelImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
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
			void ProcessNodeTree(aiNode* node, uint16_t parentIndex);
			void ConvertMaterials();
			std::filesystem::path GetTexturePath(aiMaterial* pMaterial, aiTextureType type);
			void InitSubmeshes();
			void ProcessVertices();
			void PreprocessBones();
			void ProcessVertexBoneWeights();
			void NormalizeBoneWeights();
			void ProcessAnimations();
			void AddBoneData(unsigned int vertexId, unsigned int boneId, unsigned int vertexWeight);

			void OutputPrefabs();
			void OutputMeshes();
			void OutputVertexArray(std::ofstream& output, std::vector<uint16_t>& vertexArray);
			void OutputVertexArray(std::ofstream& output, std::vector<float>& vertexArray);

			// Members
			std::filesystem::path path;
			std::filesystem::path baseFolderPath;
			std::map<std::string, glm::mat4> tempOffsetMatrices; // Save string->offset matrix so we can use it when constructing the bone data
			std::map<std::string, unsigned int> boneMapping;
			const aiScene* scene;
			bool hasExtraWeights = false;

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

			struct OutputData {
				uint32_t vertexCount = 0;
				uint32_t indexCount = 0;
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
				std::vector<Submesh> meshes;
				std::vector<BoneData> bones;
				std::vector<std::string> materialNames;
				std::vector<std::string> boneNames;
			} outputData;
		};

		void ImportModel(std::filesystem::path& path);
	}
}
