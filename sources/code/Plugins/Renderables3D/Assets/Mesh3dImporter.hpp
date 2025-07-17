#pragma once

#include <string>
#include <vector>
#include <map>

#include <Common/Formats/Model.hpp>
#include <Common/Graphics/Formats.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include "Mesh3dAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Buffer;
	}

	class EngineCore;

	class Mesh3dImporter : public SpecificAssetImporter<Mesh3dAsset, AssetType::Mesh3d> {
		public:
			Mesh3dImporter(EngineCore* engineCore);
			virtual ~Mesh3dImporter() override;

			virtual void* LoadAsset(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;
			void PrepareLayouts();
			virtual void OnDeleteAsset(Grindstone::Mesh3dAsset& asset) override;
		private:
			uint64_t GetTotalFileSize(Formats::Model::V1::Header& header);
			bool ImportModelFile(Mesh3dAsset& mesh);
			void LoadMeshImportSubmeshes(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr
			);
			void LoadMeshImportVertices(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr,
				std::vector<GraphicsAPI::Buffer*>& vertexBuffers
			);
			void LoadMeshImportIndices(
				Mesh3dAsset& mesh,
				Formats::Model::V1::Header& header,
				char*& sourcePtr,
				GraphicsAPI::Buffer*& indexBuffer
			);
		public:
			EngineCore* engineCore;
		private:
			static Grindstone::GraphicsAPI::VertexInputLayout vertexLayout;

			enum class Mesh3dLayoutIndex {
				Position = 0,
				Normal,
				Tangent,
				Uv0,
				Uv1
			};
	};
}
