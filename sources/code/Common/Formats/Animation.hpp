#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "../Math.hpp"

namespace Grindstone {
	namespace Formats {
		namespace Animation {
			namespace V1 {
				struct Header {
					uint32_t totalFileSize = 0;
					uint32_t version = 1;
					float animationDuration = 1.f;
					float ticksPerSecond = 0.25f;
					uint16_t channelCount = 0;
				};

				struct Channel {
					uint16_t boneIndex;
					uint16_t positionCount;
					uint16_t scaleCount;
					uint16_t rotationCount;
				};

				struct ChannelData {
					std::vector<PositionKeyframe> positions;
					std::vector<ScaleKeyframe> scales;
					std::vector<RotationKeyframe> rotations;
				};

				struct PositionKeyframe {
					float time;
					Math::Float3 value;
				};

				struct ScaleKeyframe {
					float time;
					Math::Float3 value;
				};

				struct RotationKeyframe {
					float time;
					Math::Quaternion value;
				};
			}
		}
	}
}
