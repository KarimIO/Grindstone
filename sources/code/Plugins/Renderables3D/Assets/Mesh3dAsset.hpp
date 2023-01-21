#pragma once

#include <vector>
#include <filesystem>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/VertexBuffer.hpp"
#include "Common/Graphics/IndexBuffer.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
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
			std::vector<std::string> materials;
		};

		GraphicsAPI::VertexArrayObject* vertexArrayObject = nullptr;
		std::vector<Submesh> submeshes;

		DEFINE_ASSET_TYPE
	};
}
