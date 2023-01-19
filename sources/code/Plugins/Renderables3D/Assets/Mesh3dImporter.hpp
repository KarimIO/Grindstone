#pragma once

#include <string>
#include <vector>
#include <map>

#include "Mesh3dAsset.hpp"
#include "Common/Formats/Model.hpp"
#include "EngineCore/Assets/AssetImporter.hpp"

namespace Grindstone {
	class Mesh3dImporter : public AssetImporter {
		public:
			Mesh3dImporter();
			virtual void* ProcessLoadedFile(Uuid uuid, std::vector<char>& contents) override;
			virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
			void PrepareLayouts();
			virtual Mesh3dAsset& LoadMesh3d(Uuid uuid);
			virtual void DecrementMeshCount(ECS::Entity entity, Uuid uuid);
		private:
			void LoadMeshImportSubmeshes(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr
			);
			void LoadMeshImportVertices(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr,
				std::vector<GraphicsAPI::VertexBuffer*>& vertexBuffers
			);
			void LoadMeshImportIndices(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr,
				GraphicsAPI::IndexBuffer*& indexBuffer
			);
			Mesh3dAsset& CreateMesh3dFromFile(Uuid uuid);
			void CreateMeshFromData(Mesh3dAsset& mesh, std::vector<char>& fileContent);
		private:
			std::map<Uuid, Mesh3dAsset> meshes;
			struct VertexLayouts {
				GraphicsAPI::VertexBufferLayout positions;
				GraphicsAPI::VertexBufferLayout normals;
				GraphicsAPI::VertexBufferLayout tangents;
				GraphicsAPI::VertexBufferLayout uv0;
				GraphicsAPI::VertexBufferLayout uv1;
			} vertexLayouts;

			enum class Mesh3dLayoutIndex {
				Position = 0,
				Normal,
				Tangent,
				Uv0,
				Uv1
			};
	};
}
