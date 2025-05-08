#pragma once

#include <filesystem>
#include <string_view>
#include <array>
#include <map>
#include <optional>

#include <Common/Graphics/Formats.hpp>
#include "ParameterType.hpp"

enum class ShaderCodeType {
	Unset,
	Glsl,
	Hlsl
};

struct ParseTree {
	struct RenderState {
		std::optional<Grindstone::GraphicsAPI::GeometryType> primitiveType;
		std::optional<Grindstone::GraphicsAPI::PolygonFillMode> polygonFillMode;
		std::optional<Grindstone::GraphicsAPI::CullMode> cullMode;

		struct AttachmentData {
			std::optional<Grindstone::GraphicsAPI::ColorMask> colorMask;

			std::optional<Grindstone::GraphicsAPI::BlendOperation> blendColorOperation;
			std::optional<Grindstone::GraphicsAPI::BlendFactor> blendColorFactorSrc;
			std::optional<Grindstone::GraphicsAPI::BlendFactor> blendColorFactorDst;

			std::optional<Grindstone::GraphicsAPI::BlendOperation> blendAlphaOperation;
			std::optional<Grindstone::GraphicsAPI::BlendFactor> blendAlphaFactorSrc;
			std::optional<Grindstone::GraphicsAPI::BlendFactor> blendAlphaFactorDst;
		};

		std::vector<AttachmentData> attachmentData;
		bool shouldCopyFirstAttachment = false;

		std::optional<Grindstone::GraphicsAPI::CompareOperation> depthCompareOp;
		std::optional<bool> isStencilEnabled;
		std::optional<bool> isDepthTestEnabled;
		std::optional<bool> isDepthWriteEnabled;
		std::optional<bool> isDepthBiasEnabled;
		std::optional<bool> isDepthClampEnabled;

		std::optional<float> depthBiasConstantFactor;
		std::optional<float> depthBiasSlopeFactor;
		std::optional<float> depthBiasClamp;
	};

	enum class ParentType : uint8_t {
		None,
		Inherit,
		Clone
	};

	struct ParentData {
		std::string parent;
		ParentType parentType = ParentType::None;
	};

	struct ShaderBlock {
		std::filesystem::path sourceFilepath;
		ShaderCodeType type = ShaderCodeType::Unset;
		std::vector<std::string> requiredShaderBlocks;
		std::string code;
		ParentData parentData;
		std::array<std::string, Grindstone::GraphicsAPI::numShaderTotalStage> stageEntryPoints;
	};

	struct Pass {
		std::filesystem::path sourceFilepath;
		bool isAbstract;
		ParentData parentData;
		RenderState renderState;
		ShaderBlock shaderBlock;
	};

	struct Configuration {
		bool isAbstract;
		std::filesystem::path sourceFilepath;
		ParentData parentData;
		std::vector<std::string_view> tags;
		std::map<std::string, Pass> passes;
	};

	struct MaterialParameter {
		ParameterType parameterType;
		std::string_view name;
		std::string_view defaultValue;
	};

	struct PipelineSet {
		std::filesystem::path sourceFilepath;
		bool isAbstract;
		ParentData parentData;
		std::vector<std::string_view> tags;
		std::vector<MaterialParameter> parameters;
		std::map<std::string, Configuration> configurations;
	};

	struct ComputeSet {
		std::filesystem::path sourceFilepath;
		ShaderBlock shaderBlock;
	};

	std::map<std::string, PipelineSet> pipelineSets;
	std::map<std::string, ComputeSet> computeSets;
	std::map<std::string, Configuration> genericConfigurations;
	std::map<std::string, Pass> genericPasses;
	std::map<std::string, ShaderBlock> genericShaderBlocks;
};
