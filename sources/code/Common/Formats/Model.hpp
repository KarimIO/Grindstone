#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

namespace Grindstone::Formats::Model {
	enum class IndexSize : uint8_t {
		Bit16,
		Bit32
	};

	namespace V1 {
		struct BoundingData {
			glm::vec3 minAABB;
			glm::vec3 maxAABB;
			glm::vec3 sphereCenter;
			float sphereRadius;
		};

		struct Header {
			uint32_t totalFileSize = 0;
			uint32_t version = 1;
			uint32_t meshCount = 0;
			uint64_t vertexCount = 0;
			uint64_t indexCount = 0;
			IndexSize isUsing32BitIndices = IndexSize::Bit16;
			bool hasVertexPositions = false;
			bool hasVertexNormals = false;
			bool hasVertexTangents = false;
			uint32_t vertexUvSetCount = 0;
			int numWeightPerBone = 4;
		};
	}
}
