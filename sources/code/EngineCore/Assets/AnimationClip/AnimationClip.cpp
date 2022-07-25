#include <vector>
#include "AnimationClip.hpp"

template <class T>
int GetCurrentKeyframeIndex(float time, std::vector<T> &keyframes) {
	for (int i = 0; i < keyframes.size() - 1; ++i) {
		if (keyframes[i + 1].time > time) {
			return i;
		}
	}

	return 0;
}

template <class T, class E>
E CalculateKeyframeData(float time, std::vector<T> &keyframes) {
	if (keyframes.size() == 1) {
		return keyframes[0].value;
	}

	int prevKeyframeIndex = GetCurrentKeyframeIndex(time, keyframes);
	auto& prevKeyframe = keyframes[prevKeyframeIndex];
	if (time > prevKeyframe.time) {
		return prevKeyframe.value;
	}

	auto& nextKeyframe = keyframes[prevKeyframeIndex + 1];
	float dt = nextKeyframe.time - prevKeyframe.time;
	float factor = (time - prevKeyframe.time) / dt;
	return glm::mix((E)prevKeyframe.value, (E)nextKeyframe.value, factor);
}

void Grindstone::AnimationClip::GetFrameMatrices(
	float time,
	Rig* rig,
	std::vector<glm::mat4>& finalTransformations
) {
	size_t boneCount = rig->bones.size();
	finalTransformations.resize(boneCount);
	std::vector<glm::mat4> modelMatrices(boneCount);

	// Apply animations
	for (size_t i = 0; i < boneCount; ++i) {
		Rig::Bone& bone = rig->bones[i];

		modelMatrices[i] = rig->bones[i].localMatrix;
	}

	for (auto& channel : channels) {
		glm::vec3 scale = CalculateScale(time, channel);
		glm::quat rotation = CalculateRotation(time, channel);
		glm::vec3 position = CalculatePosition(time, channel);

		glm::mat4 modelMatrix = 
			glm::translate(position) *
			glm::toMat4(rotation) *
			glm::scale(scale);

		Rig::Bone& bone = rig->bones[channel.boneIndex];

		modelMatrices[channel.boneIndex] =
			modelMatrices[channel.boneIndex] *
			modelMatrix;
	}

	// Apply parent matrices
	finalTransformations[0] = modelMatrices[0];

	for (size_t i = 1; i < boneCount; ++i) {
		Rig::Bone& bone = rig->bones[i];
		glm::mat4 parentMatrix = modelMatrices[bone.parentBoneIndex];

		finalTransformations[i] = parentMatrix * modelMatrices[i];
	}

	// Apply inverse matrices
	for (size_t i = 0; i < boneCount; ++i) {
		finalTransformations[i] = finalTransformations[i] * rig->bones[i].inverseModelMatrix;
	}
}

glm::vec3 Grindstone::AnimationClip::CalculatePosition(float time, Channel& channel) {
	return CalculateKeyframeData<PositionKeyframe, glm::vec3>(time, channel.positions);
}

glm::quat Grindstone::AnimationClip::CalculateRotation(float time, Channel& channel) {
	return CalculateKeyframeData<RotationKeyframe, glm::quat>(time, channel.rotations);
}

glm::vec3 Grindstone::AnimationClip::CalculateScale(float time, Channel& channel) {
	return CalculateKeyframeData<ScaleKeyframe, glm::vec3>(time, channel.scales);
}
