#pragma once

#include <string>
#include <vector>
#include <map>
#include <assimp/scene.h>

namespace Grindstone {
	namespace Converters {
		class ModelConverter {
		public:
			void Convert(const char* path);
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
			void ProcessNodeTree(aiNode* node);
			void ConvertMaterials();
			void ConvertTexture(aiMaterial* pMaterial, aiTextureType type, std::string basePath, std::string& outPath);
			void InitSubmeshes();
			void ProcessVertices();

			void OutputPrefabs();
			void OutputMeshes();
			void OutputVertexArray(std::ofstream& output, std::vector<float>& vertexArray);

			// Members
			std::string path;
			const aiScene* scene;

			struct OutputData {
				uint32_t vertexCount = 0;
				uint32_t indexCount = 0;
				struct VertexArray {
					std::vector<float> position;
					std::vector<float> normal;
					std::vector<float> tangent;
					std::vector<std::vector<float>> texCoordArray;
				} vertexArray;
				std::vector<uint16_t> indices;
				std::vector<Submesh> meshes;
				std::vector<std::string> materialNames;
			} outputData;
		};

		void ImportModel(const char* path);
	}
}
