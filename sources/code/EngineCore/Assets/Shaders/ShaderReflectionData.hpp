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
			size_t bindingId = 0;
			size_t bufferSize = 0;
			uint8_t shaderStagesBitMask = 0;
			std::vector<MemberData> members;

			StructData() = default;
			StructData(
				std::string name,
				size_t bindingId,
				size_t bufferSize,
				uint8_t shaderStagesBitMask
			) : name(name),
				bindingId(bindingId),
				bufferSize(bufferSize),
				shaderStagesBitMask(shaderStagesBitMask) {}
		};

		struct TextureData {
			std::string name;
			size_t bindingId = 0;
			uint8_t shaderStagesBitMask = 0;

			TextureData() = default;
			TextureData(
				std::string name,
				size_t bindingId,
				uint8_t shaderStagesBitMask
			) : name(name),
				bindingId(bindingId),
				shaderStagesBitMask(shaderStagesBitMask) {}
		};

		std::string name;
		std::string renderQueue;
		uint8_t shaderStagesBitMask;
		size_t numShaderStages;
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
