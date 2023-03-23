#pragma once

#include <string>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "EngineCore/Assets/Asset.hpp"

#include "Common/Formats/Animation.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	class* RigAsset;
	struct AnimationClipAsset : public Asset {
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

		struct Channel {
			size_t boneIndex;
			std::vector<PositionKeyframe> positions;
			std::vector<ScaleKeyframe> scales;
			std::vector<RotationKeyframe> rotations;
		};

		float animationDuration = 1.f;
		float ticksPerSecond = 0.25f;
		std::vector<Channel> channels;
		void GetFrameMatrices(
			float time,
			RigAsset* rig,
			std::vector<glm::mat4>& transformations
		);

		inline glm::vec3 CalculatePosition(float time, Channel& channel);
		inline glm::quat CalculateRotation(float time, Channel& channel);
		inline glm::vec3 CalculateScale(float time, Channel& channel);

		DEFINE_ASSET_TYPE
	};
}
