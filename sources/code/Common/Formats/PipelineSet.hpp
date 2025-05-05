#pragma once

#include <stdint.h>
#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/VertexBuffer.hpp>

namespace Grindstone::Formats::Pipelines::V1 {
	constexpr auto FileMagicCode = "GPSF";

	enum class PipelineType : uint8_t {
		Graphics,
		Compute
	};

	struct PassPipelineHeader {
		float depthBiasClamp;
		float depthBiasConstantFactor;
		float depthBiasSlopeFactor;
		Grindstone::GraphicsAPI::CullMode cullMode;
		Grindstone::GraphicsAPI::CompareOperation depthCompareOp;
		Grindstone::GraphicsAPI::PolygonFillMode polygonFillMode;
		Grindstone::GraphicsAPI::GeometryType primitiveType;
		uint8_t flags;
		uint8_t attachmentCount;
		uint8_t shaderStageCount;
		uint8_t descriptorSetCount;
		uint8_t descriptorBindingCount;
	};

	struct PassPipelineShaderStageHeader {
		uint32_t shaderCodeSize;
		Grindstone::GraphicsAPI::ShaderStage stageType;
	};

	struct PassPipelineAttachmentHeader {
		Grindstone::GraphicsAPI::ColorMask colorMask;
		Grindstone::GraphicsAPI::BlendFactor blendAlphaFactorDst;
		Grindstone::GraphicsAPI::BlendFactor blendAlphaFactorSrc;
		Grindstone::GraphicsAPI::BlendOperation blendAlphaOperation;
		Grindstone::GraphicsAPI::BlendFactor blendColorFactorDst;
		Grindstone::GraphicsAPI::BlendFactor blendColorFactorSrc;
		Grindstone::GraphicsAPI::BlendOperation blendColorOperation;
	};

	struct GraphicsPipelineHeader {
		uint32_t configurationCount = 0;
	};

	struct ComputePipelineHeader {
		uint32_t codeSize = 0;
	};

	struct PipelineConfigurationHeader {
		// TODO: When really supporting multiple configurations:
		// uint32_t tagCount = 0;
		uint32_t passCount = 0;
	};

	struct PassDescriptorBinding {
		uint32_t bindingIndex;
		uint32_t bindingCount;
		Grindstone::GraphicsAPI::BindingType type;
		Grindstone::GraphicsAPI::ShaderStageBit stages;
	};

	struct PassDescriptorSet {
		uint32_t set = 0;
		uint32_t bindingsOffsetFromFile = 0;
		uint32_t bindingCount = 0;
	};

	struct PassVertexBuffer {
		bool elementRate = false;
		uint32_t stride = 0;
		uint32_t attributeOffset = 0; // Offset from start of file.
		uint8_t attributeCount = 0;
	};

	struct PassVertexAttribute {
		uint32_t location = 0;
		Grindstone::GraphicsAPI::VertexFormat format = Grindstone::GraphicsAPI::VertexFormat::Float;
		uint32_t offset = 0;
		uint32_t size = 0;
		uint32_t componentsCount = 0;
		bool isNormalized = false;

		Grindstone::GraphicsAPI::AttributeUsage usage = Grindstone::GraphicsAPI::AttributeUsage::Other;
		const char* name = "";
	};

	struct PipelineSetFileHeader {
		uint8_t versionMajor = 1;
		uint8_t versionMinor = 0;
		uint8_t versionPatch = 0;
		uint8_t headerSize = sizeof(PipelineSetFileHeader);
		uint8_t graphicsPipelineSize = sizeof(GraphicsPipelineHeader);
		uint8_t computePipelineSize = sizeof(ComputePipelineHeader);
		uint8_t configurationSize = sizeof(PipelineConfigurationHeader);
		uint8_t passSize = sizeof(PassPipelineHeader);
		uint8_t attachmentSize = sizeof(PassPipelineAttachmentHeader);
		uint8_t stageSize = sizeof(PassPipelineShaderStageHeader);
		uint32_t graphicsPipelineCount = 0;
		uint32_t computePipelineCount = 0;
	};

	struct ShaderReflectInputVariables {
	};

	struct ShaderReflectDescriptorBinding {
		uint32_t bindingIndex;
		uint32_t count;
		Grindstone::GraphicsAPI::BindingType type;
		Grindstone::GraphicsAPI::ShaderStageBit stages;
	};

	struct ShaderReflectDescriptorSet {
		uint32_t setIndex;
		uint32_t firstBindingIndex;
		uint32_t bindingCount;
	};
}
