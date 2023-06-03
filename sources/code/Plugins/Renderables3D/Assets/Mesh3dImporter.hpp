#pragma once

#include <string>
#include <vector>
#include <map>

#include "Mesh3dAsset.hpp"
#include "Common/Formats/Model.hpp"
#include "EngineCore/Assets/AssetImporter.hpp"

namespace Grindstone {
	class EngineCore;

	class Mesh3dImporter : public AssetImporter {
		public:
			Mesh3dImporter(EngineCore* engineCore);
			virtual void* ProcessLoadedFile(Uuid uuid) override;
			virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
			void PrepareLayouts();
			virtual void DecrementMeshCount(ECS::Entity entity, Uuid uuid);
		private:
			uint64_t GetTotalFileSize(Formats::Model::V1::Header& header);
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
		public:
			EngineCore* engineCore;
		private:
			std::map<Uuid, Mesh3dAsset> meshes;
			struct VertexLayouts {
				GraphicsAPI::VertexBufferLayout positions;
				GraphicsAPI::VertexBufferLayout normals;
				GraphicsAPI::VertexBufferLayout tangents;
				GraphicsAPI::VertexBufferLayout uv0;
				GraphicsAPI::VertexBufferLayout uv1;
			};

			static VertexLayouts vertexLayouts;

			enum class Mesh3dLayoutIndex {
				Position = 0,
				Normal,
				Tangent,
				Uv0,
				Uv1
			};
	};
}
