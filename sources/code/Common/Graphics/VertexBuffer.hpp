#pragma once

#include <vector>
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		enum class VertexFormat {
			Float = 0,
			Float2,
			Float3,
			Float4,
			Mat3,
			Mat4,
			Int,
			Int2,
			Int3,
			Int4,
			UInt,
			UInt2,
			UInt3,
			UInt4,
			Bool
		};

		static uint32_t vertexFormatTypeSize(VertexFormat type)
		{
			switch (type)
			{
			case VertexFormat::Float:    return 4;
			case VertexFormat::Float2:   return 4 * 2;
			case VertexFormat::Float3:   return 4 * 3;
			case VertexFormat::Float4:   return 4 * 4;
			case VertexFormat::Mat3:     return 4 * 3 * 3;
			case VertexFormat::Mat4:     return 4 * 4 * 4;
			case VertexFormat::Int:      return 4;
			case VertexFormat::Int2:     return 4 * 2;
			case VertexFormat::Int3:     return 4 * 3;
			case VertexFormat::Int4:     return 4 * 4;
			case VertexFormat::UInt:     return 4;
			case VertexFormat::UInt2:    return 4 * 2;
			case VertexFormat::UInt3:    return 4 * 3;
			case VertexFormat::UInt4:    return 4 * 4;
			case VertexFormat::Bool:     return 1;
			}

			return 0;
		};

		static uint32_t vertexFormatTypeComponents(VertexFormat type)
		{
			switch (type)
			{
			case VertexFormat::Float:    return 1;
			case VertexFormat::Float2:   return 2;
			case VertexFormat::Float3:   return 3;
			case VertexFormat::Float4:   return 4;
			case VertexFormat::Mat3:     return 4 * 3;
			case VertexFormat::Mat4:     return 4 * 4;
			case VertexFormat::Int:      return 1;
			case VertexFormat::Int2:     return 2;
			case VertexFormat::Int3:     return 3;
			case VertexFormat::Int4:     return 4;
			case VertexFormat::UInt:     return 1;
			case VertexFormat::UInt2:    return 2;
			case VertexFormat::UInt3:    return 3;
			case VertexFormat::UInt4:    return 4;
			case VertexFormat::Bool:     return 1;
			}

			return 0;
		};

		enum class AttributeUsage {
			Position,
			Color,
			TexCoord0,
			TexCoord1,
			TexCoord2,
			TexCoord3,
			Normal,
			Tangent,
			BlendWeights,
			BlendIndices,
			Other
		};

		struct VertexAttributeDescription {
			uint32_t location = 0;
			VertexFormat format = VertexFormat::Float;
			uint32_t offset = 0;
			uint32_t size = 0;
			uint32_t componentsCount = 0;
			bool normalized = false;

			AttributeUsage usage = AttributeUsage::Other;
			const char *name = "";

			VertexAttributeDescription() = default;

			VertexAttributeDescription(VertexFormat _format, const char* _name, bool _normalized = false, AttributeUsage _usage = AttributeUsage::Other) :
				format(_format), name(_name), usage(_usage), size(vertexFormatTypeSize(_format)), componentsCount(vertexFormatTypeComponents(_format)), normalized(_normalized), offset(0), location(0) {

			}
		};

		struct VertexBufferLayout {
			VertexBufferLayout() = default;
			VertexBufferLayout(const std::initializer_list<VertexAttributeDescription>& elements, bool _element_rate = false)
				: attributeCount((uint32_t)elements.size()), stride(0), elementRate(_element_rate), attributes(new VertexAttributeDescription[elements.size()]) {
				memcpy(attributes, elements.begin(), sizeof(VertexAttributeDescription) * elements.size());

				for (uint32_t i = 0; i < attributeCount; ++i) {
					attributes[i].offset = stride;
					stride += attributes[i].size;
				}
			}
			uint32_t stride = 0;
			bool elementRate = false;
			VertexAttributeDescription* attributes = nullptr;
			uint32_t attributeCount = 0;
		};

		struct VertexArrayBindingLayout {
			VertexBufferLayout* vertexBindingDescriptions;
			uint32_t bindingCount;
		};

		class VertexBuffer {
		public:
			struct CreateInfo {
				VertexBufferLayout* layout;
				const void* content = 0;
				uint32_t size = 0;
				uint32_t count = 0;
			};

			virtual ~VertexBuffer() {};
		};
	};
};