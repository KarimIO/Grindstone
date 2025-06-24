#pragma once

#include <stdint.h>
#include <Common/Graphics/Formats.hpp>

namespace Grindstone::Formats::Pipelines::V1 {
	constexpr auto FileMagicCode = "GPSF";

	enum class ReflectedBlockVariableType {
		Color,
		Bool,
		Bool2,
		Bool3,
		Bool4,
		Int,
		Int2,
		Int3,
		Int4,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix2x2,
		Matrix2x3,
		Matrix2x4,
		Matrix3x2,
		Matrix3x3,
		Matrix3x4,
		Matrix4x2,
		Matrix4x3,
		Matrix4x4,
		Array,
		RuntimeArray,
		Struct
	};

	enum class PipelineType : uint8_t {
		Graphics,
		Compute
	};

	struct PassPipelineHeader {
		uint32_t pipelineNameOffsetFromBlobStart;
		uint32_t renderQueueNameOffsetFromBlobStart;
		float depthBiasClamp;
		float depthBiasConstantFactor;
		float depthBiasSlopeFactor;
		Grindstone::GraphicsAPI::CullMode cullMode;
		Grindstone::GraphicsAPI::CompareOperation depthCompareOp;
		Grindstone::GraphicsAPI::PolygonFillMode polygonFillMode;
		Grindstone::GraphicsAPI::GeometryType geometryType;
		uint8_t flags;
		uint16_t attachmentStartIndex;
		uint8_t attachmentCount;
		uint16_t shaderStageStartIndex;
		uint8_t shaderStageCount;
		uint16_t descriptorSetStartIndex;
		uint8_t descriptorSetCount;
		uint16_t descriptorBindingStartIndex;
		uint8_t descriptorBindingCount;
	};

	struct PassPipelineShaderStageHeader {
		uint32_t shaderCodeOffsetFromBlobStart;
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

	struct GraphicsPipelineSetHeader {
		uint32_t configurationStartIndex = 0;
		uint32_t configurationCount = 0;
		uint32_t materialBufferSize = 1;
	};

	struct GraphicsPipelineConfigurationHeader {
		// TODO: When really supporting multiple configurations:
		// uint32_t tagCount = 0;
		uint16_t passStartIndex = 0;
		uint16_t passCount = 0;
	};

	struct PassDescriptorBinding {
		uint32_t bindingIndex;
		uint32_t bindingArrayCount;
		Grindstone::GraphicsAPI::BindingType type;
		Grindstone::GraphicsAPI::ShaderStageBit stages;
	};

	struct PassDescriptorSet {
		uint32_t set = 0;
		uint32_t bindingsStartIndex = 0;
		uint32_t bindingCount = 0;
	};

	struct ComputePipelineConfigurationHeader {
		uint16_t shaderStageIndex;
		uint16_t descriptorSetStartIndex;
		uint8_t descriptorSetCount;
		uint16_t descriptorBindingStartIndex;
		uint8_t descriptorBindingCount;
	};

	struct ComputePipelineSetHeader {
		uint32_t configurationStartIndex = 0;
		uint32_t configurationCount = 0;
	};

	struct PipelineSetFileHeader {
		uint8_t versionMajor = 1;
		uint8_t versionMinor = 0;
		uint8_t versionPatch = 0;
		uint8_t headerSize = sizeof(PipelineSetFileHeader);
		uint8_t graphicsPipelineSize = sizeof(GraphicsPipelineSetHeader);
		uint8_t computePipelineSize = sizeof(ComputePipelineSetHeader);
		uint8_t computeConfigurationSize = sizeof(ComputePipelineConfigurationHeader);
		uint8_t graphicsConfigurationSize = sizeof(GraphicsPipelineConfigurationHeader);
		uint8_t passSize = sizeof(PassPipelineHeader);
		uint8_t attachmentSize = sizeof(PassPipelineAttachmentHeader);
		uint8_t stageSize = sizeof(PassPipelineShaderStageHeader);
		uint32_t graphicsPipelinesOffset = 0;
		uint32_t graphicsPipelineCount = 0;
		uint32_t computePipelinesOffset = 0;
		uint32_t computePipelineCount = 0;
		uint32_t materialParametersOffset = 0;
		uint32_t materialParameterCount = 0;
		uint32_t materialResourcesOffset = 0;
		uint32_t materialResourceCount = 0;
		uint32_t graphicsConfigurationsOffset = 0;
		uint32_t graphicsConfigurationCount = 0;
		uint32_t computeConfigurationsOffset = 0;
		uint32_t computeConfigurationCount = 0;
		uint32_t graphicsPassesOffset = 0;
		uint32_t graphicsPassCount = 0;
		uint32_t shaderStagesOffset = 0;
		uint32_t shaderStageCount = 0;
		uint32_t attachmentHeadersOffset = 0;
		uint32_t attachmentHeaderCount = 0;
		uint32_t descriptorSetsOffset = 0;
		uint32_t descriptorSetCount = 0;
		uint32_t descriptorBindingsOffset = 0;
		uint32_t descriptorBindingCount = 0;
		uint32_t blobSectionOffset = 0;
		uint32_t blobSectionSize = 0;
		uint32_t bufferReflectionsOffset = 0;
		uint32_t bufferReflectionsCount = 0;
		uint32_t bufferMemberReflectionOffset = 0;
		uint32_t bufferMemberReflectionCount = 0;
	};

	struct ShaderReflectInputVariables {
	};

	struct ShaderReflectDescriptorBinding {
		uint32_t bindingIndex;
		uint32_t count;
		Grindstone::GraphicsAPI::BindingType type;
		Grindstone::GraphicsAPI::ShaderStageBit stages;
		uint32_t descriptorNameOffsetFromBlobStart;
	};

	struct ShaderReflectDescriptorSet {
		uint32_t setIndex;
		uint32_t bindingStartIndex;
		uint32_t bindingCount;
	};

	// TODO: This should only really be a way to give the default value, since we have BufferReflectionMember giving the rest of the data.
	struct MaterialParameter {
		uint32_t nameOffsetFromBlobStart;
		uint32_t byteOffsetFromBufferStart;
		ReflectedBlockVariableType parameterType;
	};

	struct MaterialResource {
		Grindstone::GraphicsAPI::BindingType type;
		uint32_t nameOffsetFromBlobStart;
		uint32_t setIndex;
		uint32_t bindingIndex;
	};

	struct BufferReflection {
		uint32_t nameOffsetFromBlobStart;
		uint32_t descriptorSet;
		uint32_t descriptorBinding;
		uint32_t memberStartIndex;
		uint32_t memberCount;
		uint32_t totalSize;
	};

	struct BufferReflectionMember {
		uint32_t nameOffsetFromBlobStart;
		uint32_t byteOffsetFromBufferStart;
		uint32_t arrayCount;
		uint32_t size;
		ReflectedBlockVariableType parameterType;
	};
}
