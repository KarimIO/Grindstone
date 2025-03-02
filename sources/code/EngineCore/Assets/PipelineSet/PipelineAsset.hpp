#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace Grindstone::PipelineAssetMetaData {
	enum class ParameterType : uint8_t {
		Unknown,
		Void,
		Boolean,
		SByte,
		UByte,
		Short,
		UShort,
		Int,
		UInt,
		Int64,
		UInt64,
		AtomicCounter,
		Half,
		Float,
		Double,
		Struct,
		Image,
		SampledImage,
		Sampler,
		AccelerationStructure,
		RayQuery,
		Count
	};

	union ParameterValue {
		bool boolValue;
		int8_t int8Value;
		uint8_t uint8Value;
		int16_t int16Value;
		uint16_t uint16Value;
		int32_t int32Value;
		uint32_t uint32Value;
		int64_t int64Value;
		uint64_t uint64Value;
		short halfValue; // TODO: Make this use actual half floats
		float floatValue;
		double doubleValue;
		// uint8_t AtomicCounter;
		// Struct,
		// Image,
		// SampledImage,
		// Sampler,
		// AccelerationStructure,
		// RayQuery,
	};

	struct Parameter {
		std::string name;
		ParameterType type;
		ParameterValue defaultValue;
		size_t offset;
		size_t size;
	};

	enum class DefaultTexture : uint8_t {
		White,
		Black,
		Normal
	};

	struct Buffer {
		size_t bufferSize;
		std::vector<Parameter> parameters;
	};

	struct TextureSlot {
		std::string slotName;
		DefaultTexture defaultTexture;
	};
}
