#pragma once

#include <string>
#include "EngineCore/ECS/Entity.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"
#include "Common/Formats/Animation.hpp"

namespace Grindstone {
	struct AnimationClip {
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
			std::vector<PositionKeyframe> positions;
			std::vector<ScaleKeyframe> scales;
			std::vector<RotationKeyframe> rotations;
		};

		float animationDuration = 1.f;
		float ticksPerSecond = 0.25f;
		std::vector<Channel> channels;
		void GetFrameMatrices(float time, std::vector<glm::mat4>& transformations);
		void GetFrameComponents(
			float time,
			std::vector<glm::vec3>& bonePositions,
			std::vector<glm::quat>& boneRotations,
			std::vector<glm::vec3>& boneScales
		);
		glm::vec3 CalculatePosition(float time, Channel& channel);
		glm::quat CalculateRotation(float time, Channel& channel);
		glm::vec3 CalculateScale(float time, Channel& channel);
	};
}

void Grindstone::AnimationClip::GetFrameMatrices(
	float time,
	std::vector<glm::mat4>& boneTransformations
) {
}

void Grindstone::AnimationClip::GetFrameComponents(
	float time,
	std::vector<glm::vec3>& bonePositions,
	std::vector<glm::quat>& boneRotations,
	std::vector<glm::vec3>& boneScales
) {
	for (auto& channel : channels) {
		CalculatePosition(time, channel);
		CalculateRotation(time, channel);
		CalculateScale(time, channel);
	}
}

glm::vec3 Grindstone::AnimationClip::CalculatePosition(float time, Channel& channel) {

}

glm::quat Grindstone::AnimationClip::CalculateRotation(float time, Channel& channel) {

}

glm::vec3 Grindstone::AnimationClip::CalculateScale(float time, Channel& channel) {
	for (size_t i = 0; i < channel.scales.size(); ++i) {
		if (time > channel.scales[i].time) {
		}
	}
}

namespace Grindstone {
	struct AnimationComponent {
		std::vector<glm::mat4> boneTransformations;
		AnimationClip* animationClip;
		float time;
	};

	void AnimationSystem(AnimationComponent animationComponent) {
		animationComponent.time += 1.0f / 60.0f;
		animationComponent.animationClip->GetFrameMatrices(
			animationComponent.time,
			animationComponent.boneTransformations
		);
	}
}
