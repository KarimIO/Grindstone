#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <xkeycheck.h>

#include <Common/Hash.hpp>
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class DescriptorSetLayout;

	/*! Pipelines are a program that runs on the GPU. Graphics Pipelines are a variety
		of Pipeline that deal strictly with computer graphics. They have several stages,
		some of which are programmable, to draw primitive shapes to pixels on the screen.
	*/
	class GraphicsPipeline {
	public:
		struct ShaderStageData {
			const char* fileName;
			const char* content;
			uint32_t size;
			ShaderStage type;
		};

		struct AttachmentData {
			BlendData blendData = BlendData::NoBlending();
			ColorMask colorMask = ColorMask::RGBA;
		};

		struct PipelineData {
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

			AttachmentData* colorAttachmentData = nullptr;
			uint8_t colorAttachmentCount;
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

		struct CreateInfo {
			VertexInputLayout vertexInputLayout;
			PipelineData pipelineData;
		};
	};
}

namespace std {
	template<>
	struct std::hash<Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData> {
		std::size_t operator()(const Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData& stage) const noexcept {
			size_t result = stage.size;
			result ^= std::hash<uint8_t>{}(static_cast<uint8_t>(stage.type));

			for (uint32_t i = 0; i < stage.size; ++i) {
				result ^= std::hash<uint8_t>{}(stage.content[i]);
			}

			return result;
		}
	};

	template<>
	struct std::hash<Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData> {
		std::size_t operator()(const Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData& attachment) const noexcept {
			size_t result =
				static_cast<size_t>(attachment.colorMask) |
				static_cast<size_t>(attachment.blendData.alphaFactorDst) << 8 |
				static_cast<size_t>(attachment.blendData.alphaFactorSrc) << 16 |
				static_cast<size_t>(attachment.blendData.alphaOperation) << 24 |
				static_cast<size_t>(attachment.blendData.colorFactorDst) << 32 |
				static_cast<size_t>(attachment.blendData.colorFactorSrc) << 40 |
				static_cast<size_t>(attachment.blendData.colorOperation) << 48;
			return result;
		}
	};

	template<>
	struct std::hash<Grindstone::GraphicsAPI::GraphicsPipeline::PipelineData> {
		std::size_t operator()(const Grindstone::GraphicsAPI::GraphicsPipeline::PipelineData& pipelineData) const noexcept {
			size_t result =
				static_cast<size_t>(pipelineData.cullMode) |
				static_cast<size_t>(pipelineData.depthCompareOp) << 8 |
				static_cast<size_t>(pipelineData.primitiveType) << 16 |
				static_cast<size_t>(pipelineData.polygonFillMode) << 24 |
				static_cast<size_t>(pipelineData.isDepthTestEnabled ? 1 : 0) << 32 |
				static_cast<size_t>(pipelineData.isDepthWriteEnabled ? 1 : 0) << 33 |
				static_cast<size_t>(pipelineData.isStencilEnabled ? 1 : 0) << 34 |
				static_cast<size_t>(pipelineData.hasDynamicViewport ? 1 : 0) << 35 |
				static_cast<size_t>(pipelineData.hasDynamicScissor ? 1 : 0) << 36 |
				static_cast<size_t>(pipelineData.isDepthBiasEnabled ? 1 : 0) << 37 |
				static_cast<size_t>(pipelineData.isDepthClampEnabled ? 1 : 0) << 38;

			result ^= static_cast<size_t>(pipelineData.width) | (static_cast<size_t>(pipelineData.height) << 32);
			result ^= static_cast<size_t>(pipelineData.scissorX) | (static_cast<size_t>(pipelineData.scissorY) << 32);
			result ^= static_cast<size_t>(pipelineData.scissorW) | (static_cast<size_t>(pipelineData.scissorH) << 32);
			result ^= static_cast<size_t>(pipelineData.depthBiasConstantFactor) | (static_cast<size_t>(pipelineData.depthBiasSlopeFactor) << 32);
			result ^= static_cast<size_t>(pipelineData.depthBiasClamp);

			result ^= pipelineData.colorAttachmentCount;
			for (uint8_t i = 0; i < pipelineData.colorAttachmentCount; ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData>{}(pipelineData.colorAttachmentData[i]);
			}

			result ^= pipelineData.shaderStageCreateInfoCount;
			for (uint8_t i = 0; i < pipelineData.shaderStageCreateInfoCount; ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::GraphicsPipeline::ShaderStageData>{}(pipelineData.shaderStageCreateInfos[i]);
			}

			return result;
		}
	};
}
