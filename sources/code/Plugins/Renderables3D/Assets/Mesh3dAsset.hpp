#pragma once

#include <vector>
#include <filesystem>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Formats/Model.hpp>

#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/Assets/Asset.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class VertexArrayObject;
	}

	enum class VertexBuffers {
		Vertex = 0,
		Normal,
		Tangent,
		TexCoord,
		Last
	};

	struct Mesh3dAsset : public Asset {
		Mesh3dAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}
		struct Submesh {
			uint32_t indexCount = 0;
			uint32_t baseVertex = 0;
			uint32_t baseIndex = 0;
			uint32_t materialIndex = UINT32_MAX;
			GraphicsAPI::VertexArrayObject* vertexArrayObject = nullptr;
		};

		Grindstone::Formats::Model::V1::BoundingData boundingData;
		GraphicsAPI::VertexArrayObject* vertexArrayObject = nullptr;
		std::vector<Submesh> submeshes;

		DEFINE_ASSET_TYPE("Mesh3dAsset", AssetType::Mesh3d)
	};
}
