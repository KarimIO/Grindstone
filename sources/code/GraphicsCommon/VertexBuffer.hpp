#pragma once

#include <vector>

namespace Grindstone {
	namespace GraphicsAPI {
		enum class VertexFormat {
			R32_G32 = 0,
			R32_G32_B32,
		};

		enum class AttributeUsage {
			Position,
			TexCoord0,
			TexCoord1,
			Normal,
			Tangent,
		};

		struct VertexBindingDescription {
			uint32_t binding;
			uint32_t stride;
			bool elementRate;
		};

		struct VertexAttributeDescription {
			uint32_t binding;
			uint32_t location;
			VertexFormat format;
			uint32_t offset;
			uint32_t size;
			AttributeUsage usage;
			const char *name;
		};

		struct VertexBufferCreateInfo {
			VertexBindingDescription *binding;
			uint32_t bindingCount;
			VertexAttributeDescription *attribute;
			uint32_t attributeCount;
			const void *content;
			uint32_t size;
			uint32_t count;
		};

		class VertexBuffer {
		public:
			virtual ~VertexBuffer() {};
		};
	};
};