#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "../Math.hpp"

namespace Grindstone::Formats::Animation::V1 {
	enum class KeyframeInterpolation : uint8_t {
		Step,
		Linear,
		Cubic,
	};

	struct Header {
		uint32_t totalFileSize = 0;
		uint32_t version = 1;
		double animationDuration = 1.f;
		double ticksPerSecond = 0.25f;
		uint16_t boneChannelCount = 0;
		uint16_t propertyChannelCount = 0;
		uint16_t eventCount = 0;
		uint64_t boneChannelDataOffset = 0;
		uint64_t propertyChannelDataOffset = 0;
		uint64_t positionKeyframesOffset = 0;
		uint64_t rotationKeyframesOffset = 0;
		uint64_t scaleKeyframesOffset = 0;
		uint64_t propertyKeyframesOffset = 0;
		uint64_t eventsArrayOffset = 0;
		uint64_t eventsPayloadOffset = 0;
		uint64_t stringBlockOffset = 0;
	};

	struct BoneChannel {
		uint16_t boneIndex;
		uint16_t positionCount;
		uint16_t scaleCount;
		uint16_t rotationCount;
		uint32_t positionKeyOffset = 0;
		uint32_t rotationKeyOffset = 0;
		uint32_t scaleKeyOffset = 0;
		KeyframeInterpolation interpolation = KeyframeInterpolation::Linear;
	};

	template<typename T>
	struct Keyframe {
		double time;
		T value;

		Keyframe() = default;
		Keyframe(const Keyframe& other) = default;
		Keyframe(Keyframe&& other) noexcept = default;
		Keyframe& operator=(const Keyframe& other) = default;
		Keyframe& operator=(Keyframe&& other) noexcept = default;
		Keyframe(double time, T value) : time(time), value(value) {}
	};

	struct BoneChannelData {
		std::vector<Keyframe<Grindstone::Math::Float3>> positions;
		std::vector<Keyframe<Grindstone::Math::Float3>> scales;
		std::vector<Keyframe<Grindstone::Math::Quaternion>> rotations;
	};
}
