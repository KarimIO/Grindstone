#pragma once

#include <string>
#include <vector>

namespace Grindstone {
	struct ShaderReflectionData {
		struct StructData {
			struct MemberData {
				std::string name;
				std::string baseType;
				size_t offset;
				size_t memberSize;

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
			size_t bindingId;
			size_t bufferSize;
			uint8_t shaderStagesBitMask;
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

		std::string name;
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
	};
}
