#pragma once

#include <vector>
#include <filesystem>
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/VertexBuffer.hpp"
#include "Common/Graphics/IndexBuffer.hpp"

namespace Grindstone {
	struct Vertex {
		float positions[3];
		float normal[3];
		float tangent[3];
		float texCoord[2];
	};

	enum class VertexBuffers {
		Vertex = 0,
		Normal,
		Tangent,
		TexCoord,
		Last
	};

	struct Mesh3d {
		struct Submesh {
			uint32_t indexCount = 0;
			uint32_t baseVertex = 0;
			uint32_t baseIndex = 0;
			uint32_t materialIndex = UINT32_MAX;
			Mesh3d* mesh = nullptr;
		};
		
		std::filesystem::path path;
		GraphicsAPI::VertexArrayObject* vertexArrayObject = nullptr;
		std::vector<Submesh> submeshes;
	};
}
