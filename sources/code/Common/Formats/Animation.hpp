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
					double animationDuration = 1.f;
					double ticksPerSecond = 0.25f;
					uint16_t channelCount = 0;
				};

				struct Channel {
					uint16_t boneIndex;
					uint16_t positionCount;
					uint16_t scaleCount;
					uint16_t rotationCount;
				};

				struct PositionKeyframe {
					double time;
					Math::Float3 value;

					PositionKeyframe(double time, Math::Float3 value) {
						this->time = time;
						this->value = value;
					}
				};

				struct ScaleKeyframe {
					double time;
					Math::Float3 value;

					ScaleKeyframe(double time, Math::Float3 value) {
						this->time = time;
						this->value = value;
					}
				};

				struct RotationKeyframe {
					double time;
					Math::Quaternion value;

					RotationKeyframe(double time, Math::Quaternion value) {
						this->time = time;
						this->value = value;
					}
				};

				struct ChannelData {
					std::vector<PositionKeyframe> positions;
					std::vector<ScaleKeyframe> scales;
					std::vector<RotationKeyframe> rotations;
				};
			}
		}
	}
}
