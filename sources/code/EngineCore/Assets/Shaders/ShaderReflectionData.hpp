#pragma once

#include <string>
#include <vector>

namespace Grindstone {
	struct ShaderReflectionData {
		struct StructData {
			struct MemberData {
				std::string name;
				std::string baseType;
				size_t offset = 0;
				size_t memberSize = 0;

				MemberData() = default;
				MemberData(
					std::string name,
					std::string baseType,
					size_t offset,
					size_t memberSize
				) : name(name),
					baseType(baseType),
					offset(offset),
					memberSize(memberSize) {}
			};
			std::string name;
			uint32_t bindingId = 0;
			uint32_t setId = 0;
			uint32_t bufferSize = 0;
			uint8_t shaderStagesBitMask = 0;
			std::vector<MemberData> members;

			StructData() = default;
			StructData(
				std::string name,
				uint32_t bindingId,
				uint32_t setId,
				uint32_t bufferSize,
				uint8_t shaderStagesBitMask
			) : name(name),
				bindingId(bindingId),
				setId(setId),
				bufferSize(bufferSize),
				shaderStagesBitMask(shaderStagesBitMask) {}
		};

		struct TextureData {
			std::string name;
			uint32_t bindingId = 0;
			uint32_t setId = 0;
			uint8_t shaderStagesBitMask = 0;

			TextureData() = default;
			TextureData(
				std::string name,
				uint32_t bindingId,
				uint32_t setId,
				uint8_t shaderStagesBitMask
			) : name(name),
				bindingId(bindingId),
				setId(setId),
				shaderStagesBitMask(shaderStagesBitMask) {}
		};

		std::string name;
		std::string renderQueue;
		std::string geometryRenderer;
		std::string transparencyMode;
		std::string cullMode;
		uint8_t shaderStagesBitMask = 0;
		size_t numShaderStages = 0;
		ShaderReflectionData() = default;
		ShaderReflectionData(
			std::string name,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages
		) : name(name),
		shaderStagesBitMask(shaderStagesBitMask),
		numShaderStages(numShaderStages) {}

		std::vector<StructData> uniformBuffers;
		std::vector<TextureData> textures;
	};
}
