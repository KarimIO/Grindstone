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

		class GraphicsPipeline {
		public:
			struct CreateInfo {
				const char* debugName;
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
				BlendMode blendMode;
				uint32_t colorAttachmentCount;
				bool isDepthTestEnabled = true;
				bool isDepthWriteEnabled = true;
				bool isStencilEnabled = false;
				bool hasDynamicViewport = false;
				bool hasDynamicScissor = false;
				bool isDepthBiasEnabled = false;
			};

			virtual void Recreate(CreateInfo& createInfo) = 0;
		};
	}
}
