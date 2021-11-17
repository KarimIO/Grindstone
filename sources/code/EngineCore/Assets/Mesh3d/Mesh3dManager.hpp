#pragma once

#include <string>
#include <vector>
#include <map>

#include "Mesh3d.hpp"
#include "Common/Formats/Model.hpp"

namespace Grindstone {
	class Mesh3dManager {
		public:
			Mesh3dManager();
			void PrepareLayouts();
			virtual Mesh3d& LoadMesh3d(Uuid uuid);
			virtual void DecrementMeshCount(ECS::Entity entity, Uuid uuid);
		private:
			bool TryGetMesh3d(Uuid uuid, Mesh3d*& mesh3d);
			void LoadMeshImportSubmeshes(
				Mesh3d& mesh,
				Formats::Model::Header::V1& header,
				char*& sourcePtr
			);
			void LoadMeshImportVertices(
				Mesh3d& mesh,
				Formats::Model::Header::V1& header,
				char*& sourcePtr,
				std::vector<GraphicsAPI::VertexBuffer*>& vertexBuffers
			);
			void LoadMeshImportIndices(
				Mesh3d& mesh,
				Formats::Model::Header::V1& header,
				char*& sourcePtr,
				GraphicsAPI::IndexBuffer*& indexBuffer
			);
			Mesh3d& CreateMesh3dFromFile(Uuid uuid);
			void CreateMeshFromData(Mesh3d& mesh, std::vector<char>& fileContent);
		private:
			std::map<Uuid, Mesh3d> meshes;
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
