#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
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
			PolygonFillMode polygonFillMode;
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

			BlendData blendData = BlendData::NoBlending();
			ColorMask colorMask = ColorMask::RGBA;

			uint32_t colorAttachmentCount;
			CompareOperation depthCompareOp = CompareOperation::LessOrEqual;
			bool isDepthTestEnabled = true;
			bool isDepthWriteEnabled = true;
			bool isStencilEnabled = false;
			bool hasDynamicViewport = false;
			bool hasDynamicScissor = false;
			bool isDepthBiasEnabled = false;
			bool isDepthClampEnabled = false;

			float depthBiasConstantFactor = 1.25f;
			float depthBiasSlopeFactor = 1.75f;
			float depthBiasClamp = 0.0f;
		};

		virtual void Recreate(CreateInfo& createInfo) = 0;
	};
}
