#include "pch.hpp"
#include <Plugins/Materials/MaterialManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <glm/glm.hpp>
#include <iostream>

#include "StaticMeshManager.hpp"
#include "StaticMeshV1Loader.hpp"

namespace Grindstone {
	StaticMeshManager::StaticMeshManager() {
		GraphicsAPI::VertexBufferLayout vbd({
			{ GraphicsAPI::VertexFormat::Float3, "vPosition",	false, GraphicsAPI::AttributeUsage::Position },
			{ GraphicsAPI::VertexFormat::Float3, "vNormal",		false, GraphicsAPI::AttributeUsage::Normal },
			{ GraphicsAPI::VertexFormat::Float3, "vTangent",	false, GraphicsAPI::AttributeUsage::Tangent },
			{ GraphicsAPI::VertexFormat::Float4, "vColor",		false, GraphicsAPI::AttributeUsage::Color },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord0",	false, GraphicsAPI::AttributeUsage::TexCoord0 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord1",	false, GraphicsAPI::AttributeUsage::TexCoord1 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord2",	false, GraphicsAPI::AttributeUsage::TexCoord2 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord3",	false, GraphicsAPI::AttributeUsage::TexCoord3 },
			{ GraphicsAPI::VertexFormat::Float4, "vBlendWeights",false, GraphicsAPI::AttributeUsage::BlendWeights },
			{ GraphicsAPI::VertexFormat::Float4, "vBlendIndices",false, GraphicsAPI::AttributeUsage::BlendIndices }
		});
	}

	bool StaticMeshManager::loadMeshImpl(const char* path) {
		std::string fullpath = std::string("../assets/") + path;
		std::ifstream input(fullpath, std::ios::ate | std::ios::binary);

		if (!input.is_open()) {
			//GRIND_ERROR("Failed to open file: {0}!", path);
			return false;
		}

		size_t fileSize = (size_t)input.tellg();
		std::vector<char> buffer(fileSize);

		input.seekg(0);
		input.read(buffer.data(), fileSize);

		if (fileSize < 4) {
			// GRIND_ERROR("File is too small!");
			return false;
		}

		if (buffer[0] != 'G' || buffer[1] != 'M' || buffer[2] != 'F') {
			// GRIND_ERROR("Failed to open file: {0}!", path);
			return false;
		}

		char* filePtr = buffer.data() + 3;

		uint8_t version;
		memcpy(&version, filePtr, sizeof(uint8_t));

		staticMeshMap[fullpath] = staticMeshes.size();
		staticMeshes.emplace_back();
		StaticMesh &staticMesh = staticMeshes.back();

		try {
			switch (version) {
			case 1:
				StaticMeshV1Loader loader;
				loader.processFile(filePtr, fileSize, staticMesh);
			default:
				throw "Invalid version number!";
			}
		}
		catch(std::runtime_error &e) {
			std::cerr << e.what();
			input.close();
			return false;
		};

		return true;
	};
}
