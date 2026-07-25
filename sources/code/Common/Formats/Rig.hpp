#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "../Math.hpp"

namespace Grindstone::Formats::Rig::V1 {
	const uint32_t version = 1;
	const char magicCode[5] = "GRIG";
	const uint32_t magicSize = 4;

	struct Header {
		uint64_t totalFileSize = 0;
		uint32_t version = 1;
		uint32_t bonesCount = 0;
		uint64_t boneDataOffset = 0;
		uint64_t stringBlockSize = 0;
		uint64_t stringBlockOffset = 0;
		Math::Matrix4 globalInverseTransform;
	};

	struct Bone {
		uint32_t boneNameStringOffset = 0;
		uint32_t boneParentIndex;
		Math::Matrix4 localBindTransform;
		Math::Matrix4 inverseBindTransform;
	};
}
