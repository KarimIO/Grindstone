#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
		class DescriptorSetLayout;
		struct VertexBufferLayout;

		struct ShaderStageCreateInfo {
			const char *fileName;
			const char *content;
			uint32_t size;
			ShaderStage type;
		};

		enum class CullMode {
			None = 0,
			Front,
			Back,
			Both
		};

		class Pipeline {
		public:
			struct CreateInfo {
				const char* shaderName;
				GeometryType primitiveType;
				CullMode cullMode;
				RenderPass* renderPass;
				float width, height;
				int32_t scissorX = 0, scissorY = 0;
				uint32_t scissorW, scissorH;
				ShaderStageCreateInfo* shaderStageCreateInfos;
				uint32_t shaderStageCreateInfoCount;
				DescriptorSetLayout** descriptorSetLayouts;
				uint32_t descriptorSetLayoutCount;
				VertexBufferLayout* vertexBindings;
				uint32_t vertexBindingsCount;
			};

			virtual void Recreate(CreateInfo& createInfo) = 0;
		};
	}
}
