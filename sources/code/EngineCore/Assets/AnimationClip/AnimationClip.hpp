#pragma once

#include <string>
#include "EngineCore/ECS/Entity.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"
#include "Common/Formats/Animation.hpp"
#include "Common/Math.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Grindstone {
	struct Rig {
		struct Bone {
			size_t parentBoneIndex;
			glm::mat4 localMatrix;
			glm::mat4 inverseModelMatrix;
		};

		std::vector<Bone> bones;
	};

	struct AnimationClip {
		struct PositionKeyframe {
			float time;
			Math::Float3 value;
		};

		struct ScaleKeyframe {
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
		Rig* rig;
		void GetFrameMatrices(float time, std::vector<glm::mat4>& transformations);
		void GetFrameComponents(
			float time,
			std::vector<glm::vec3>& bonePositions,
			std::vector<glm::quat>& boneRotations,
			std::vector<glm::vec3>& boneScales
		);
		inline glm::vec3 CalculatePosition(float time, Channel& channel);
		inline glm::quat CalculateRotation(float time, Channel& channel);
		inline glm::vec3 CalculateScale(float time, Channel& channel);
	};
}
