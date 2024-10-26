#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class DescriptorSetLayout;
	struct VertexBufferLayout;

	/*! Pipelines are a program that runs on the GPU. Graphics Pipelines are a variety
		of Pipeline that deal strictly with computer graphics. They have several stages,
		some of which are programmable, to draw primitive shapes to pixels on the screen.
	*/
	class GraphicsPipeline {
	public:
		struct CreateInfo {
			struct ShaderStageData {
				const char* fileName;
				const char* content;
				uint32_t size;
				ShaderStage type;
			};

			const char* debugName;
			GeometryType primitiveType;
			PolygonFillMode polygonFillMode;
			CullMode cullMode;
			RenderPass* renderPass;
			float width, height;
			int32_t scissorX = 0, scissorY = 0;
			uint32_t scissorW, scissorH;
			ShaderStageData* shaderStageCreateInfos;
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

		virtual void Recreate(const CreateInfo& createInfo) = 0;
	};
}
