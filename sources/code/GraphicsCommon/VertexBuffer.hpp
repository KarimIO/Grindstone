#pragma once

#include <vector>

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
			Normal,
			Tangent,
			Other
		};

		struct VertexAttributeDescription {
			uint32_t location;
			VertexFormat format;
			uint32_t offset;
			uint32_t size;
			uint32_t components_count;
			bool normalized;

			AttributeUsage usage;
			const char *name;

			VertexAttributeDescription() {}

			VertexAttributeDescription(VertexFormat _format, const char* _name, bool _normalized = false, AttributeUsage _usage = AttributeUsage::Other) :
				format(_format), name(_name), usage(_usage), size(vertexFormatTypeSize(_format)), components_count(vertexFormatTypeComponents(_format)), normalized(_normalized), offset(0), location(0) {

			}
		};

		struct VertexBufferLayout {
			VertexBufferLayout() : stride(0), element_rate(false), attributes(0), attribute_count(0) {}
			VertexBufferLayout(const std::initializer_list<VertexAttributeDescription>& elements, bool _element_rate = false)
				: attribute_count((uint32_t)elements.size()), stride(0), element_rate(_element_rate), attributes(new VertexAttributeDescription[elements.size()]) {
				memcpy(attributes, elements.begin(), sizeof(VertexAttributeDescription) * elements.size());

				for (uint32_t i = 0; i < attribute_count; ++i) {
					attributes[i].location = i;
					attributes[i].offset = stride;
					stride += attributes[i].size;
				}
			}
			uint32_t stride;
			bool element_rate;
			VertexAttributeDescription* attributes;
			uint32_t attribute_count;
		};

		struct VertexArrayBindingLayout {
			VertexBufferLayout* vertex_binding_descriptions;
			uint32_t binding_count;
		};

		struct VertexBufferCreateInfo {
			VertexBufferLayout *layout;
			const void *content = 0;
			uint32_t size = 0;
			uint32_t count = 0;
		};

		class VertexBuffer {
		public:
			virtual void updateBuffer(void *) = 0;
			virtual ~VertexBuffer() {};
		};
	};
};