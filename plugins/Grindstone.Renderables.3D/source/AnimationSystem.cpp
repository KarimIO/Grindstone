#include <Common/Containers/Span.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Logger.hpp>

#include <Grindstone.Renderables.3D/include/AnimationSystem.hpp>
#include <Grindstone.Renderables.3D/include/Components/AnimatorComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::Containers;

using AnimationTime = double;
using AnimationWeight = float;

const uint16_t invalidBoneIndex = std::numeric_limits<uint16_t>::max();

[[nodiscard]] static AnimationWeight GetKeyframeWeight(AnimationTime lastTimeStamp, AnimationTime nextTimeStamp, AnimationTime animationTime) {
	AnimationTime midWayLength = animationTime - lastTimeStamp;
	AnimationTime framesDiff = nextTimeStamp - lastTimeStamp;
	AnimationTime scaleFactor = midWayLength / framesDiff;
	return static_cast<AnimationWeight>(scaleFactor);
}

template<typename T>
[[nodiscard]] static size_t GetKeyframeIndexByTime(const Span<const AnimationClipAsset::Keyframe<T>> keyframes, AnimationTime animationTime) {
	// Assumes keyframes.GetSize() > 1

	for (size_t index = 0; index < keyframes.GetSize() - 1; ++index) {
		if (animationTime < keyframes[index + 1].time) {
			return index;
		}
	}

	// TODO: Extrapolation and looping?
	GS_ASSERT_LOG("Invalid keyframe");

	// After the last frame, just use the last two keyframes.
	return keyframes.GetSize() - 2;
}

[[nodiscard]] static glm::mat4 InterpolateStepPosition(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::PositionKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		return glm::translate(glm::mat4(1.0f), keyframes[0].value);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	glm::vec3 finalPosition = keyframes[lastKeyframeIndex].value;
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

[[nodiscard]] static glm::mat4 InterpolateStepRotation(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::RotationKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		// TODO: Can we get away with normalizing rotations in the importer instead?
		auto rotation = glm::normalize(keyframes[0].value);
		return glm::toMat4(rotation);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	glm::quat finalRotation = keyframes[lastKeyframeIndex].value;
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);
}

[[nodiscard]] static glm::mat4 InterpolateStepScaling(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::ScaleKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		return glm::scale(glm::mat4(1.0f), keyframes[0].value);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	glm::vec3 finalScale = keyframes[lastKeyframeIndex].value;
	return glm::scale(glm::mat4(1.0f), finalScale);
}

[[nodiscard]] static glm::mat4 InterpolateLinearPosition(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::PositionKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		return glm::translate(glm::mat4(1.0f), keyframes[0].value);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	size_t nextKeyframeIndex = lastKeyframeIndex + 1;
	AnimationWeight weight = GetKeyframeWeight(keyframes[lastKeyframeIndex].time, keyframes[nextKeyframeIndex].time, animationTime);
	glm::vec3 finalPosition = glm::mix(keyframes[lastKeyframeIndex].value, keyframes[nextKeyframeIndex].value, weight);
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

[[nodiscard]] static glm::mat4 InterpolateLinearRotation(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::RotationKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		auto rotation = glm::normalize(keyframes[0].value);
		return glm::toMat4(rotation);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	size_t nextKeyframeIndex = lastKeyframeIndex + 1;
	AnimationWeight weight = GetKeyframeWeight(keyframes[lastKeyframeIndex].time, keyframes[nextKeyframeIndex].time, animationTime);
	glm::quat finalRotation = glm::slerp(keyframes[lastKeyframeIndex].value, keyframes[nextKeyframeIndex].value, weight);
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);
}

[[nodiscard]] static glm::mat4 InterpolateLinearScaling(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::ScaleKeyframe> keyframes,
	AnimationTime animationTime
) {
	if (keyframes.GetSize() == 1) {
		return glm::scale(glm::mat4(1.0f), keyframes[0].value);
	}

	size_t lastKeyframeIndex = GetKeyframeIndexByTime(keyframes, animationTime);
	size_t nextKeyframeIndex = lastKeyframeIndex + 1;
	AnimationWeight weight = GetKeyframeWeight(keyframes[lastKeyframeIndex].time, keyframes[nextKeyframeIndex].time, animationTime);
	glm::vec3 finalScale = glm::mix(keyframes[lastKeyframeIndex].value, keyframes[nextKeyframeIndex].value, weight);
	return glm::scale(glm::mat4(1.0f), finalScale);
}

[[nodiscard]] static glm::mat4 InterpolateBoneLocalTransform(
	const AnimationClipAsset::KeyframeInterpolation interpolation,
	const Span<const AnimationClipAsset::PositionKeyframe> positionKeyframes,
	const Span<const AnimationClipAsset::RotationKeyframe> rotationKeyframes,
	const Span<const AnimationClipAsset::ScaleKeyframe> scaleframes,
	AnimationTime animationTime
) {
	switch (interpolation) {
	case AnimationClipAsset::KeyframeInterpolation::Step: {
		glm::mat4 translation = InterpolateStepPosition(interpolation, positionKeyframes, animationTime);
		glm::mat4 rotation = InterpolateStepRotation(interpolation, rotationKeyframes, animationTime);
		glm::mat4 scale = InterpolateStepScaling(interpolation, scaleframes, animationTime);
		return translation * rotation * scale;
	}
	case AnimationClipAsset::KeyframeInterpolation::Linear: {
		glm::mat4 translation = InterpolateLinearPosition(interpolation, positionKeyframes, animationTime);
		glm::mat4 rotation = InterpolateLinearRotation(interpolation, rotationKeyframes, animationTime);
		glm::mat4 scale = InterpolateLinearScaling(interpolation, scaleframes, animationTime);
		return translation * rotation * scale;
	}
	default:
		GS_ASSERT_LOG("Invalid interpolation type.");
		return glm::mat4(1.0f);
	};
}

void Grindstone::AnimateSkeletonSystem(Grindstone::WorldContextSet& worldContextSet) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	const double currentTime = engineCore.GetTimeSinceLaunch();

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	auto view = registry.view<Grindstone::AnimatorComponent>();
	view.each(
		[currentTime, &engineCore](Grindstone::AnimatorComponent& animatorComponent) {
			AssetReference<AnimationClipAsset> animationRef = animatorComponent.animation;
			AssetReference<RigAsset> rigAssetRef = animatorComponent.rig;
			if (!rigAssetRef.IsValid() || !animationRef.IsValid()) {
				return;
			}

			const RigAsset* rig = rigAssetRef.Get();
			const AnimationClipAsset* animation = animationRef.Get();

			if (rig == nullptr || animation == nullptr) {
				return;
			}

			std::vector<glm::mat4> boneMatrices;
			boneMatrices.reserve(rig->bones.size());

			for (const RigAsset::Bone& bone : rig->bones) {
				boneMatrices.push_back(bone.localMatrix);
			}

			GS_ASSERT_ENGINE(rig->bones.size() == rig->boneNameToIndex.size());

			// This requires channels to be in order where all parent nodes are before their children.
			for (const AnimationClipAsset::BoneChannel& channel : animation->boneChannels) {
				auto boneIt = rig->boneNameToIndex.find(channel.boneName);
				if (boneIt == rig->boneNameToIndex.end()) {
					// TODO: Throttle this warning or preprocess it.
					GPRINT_WARN_V(Grindstone::LogSource::Rendering, "Bone channel for bone name \"{}\" does not exist in RigAsset \"{}\". This bone channel will be ignored.", channel.boneName, rig->name);
					continue;
				}

				size_t boneIndex = boneIt->second;
				GS_ASSERT_ENGINE_WITH_MESSAGE(boneIndex < rig->bones.size(), "Bone index is out of bounds for RigAsset bones vector.");

				const RigAsset::Bone& bone = rig->bones[boneIndex];

				const AnimationClipAsset::PositionKeyframe* positionBegin = &animation->positions[channel.positionKeyOffset];
				const AnimationClipAsset::RotationKeyframe* rotationBegin = &animation->rotations[channel.rotationKeyOffset];
				const AnimationClipAsset::ScaleKeyframe* scaleBegin = &animation->scales[channel.scaleKeyOffset];

				const Span<const AnimationClipAsset::PositionKeyframe> positionSpan(positionBegin, channel.positionCount);
				const Span<const AnimationClipAsset::RotationKeyframe> rotationSpan(rotationBegin, channel.rotationCount);
				const Span<const AnimationClipAsset::ScaleKeyframe> scaleSpan(scaleBegin, channel.scaleCount);

				// This is not supported yet because Assimp does not support tangent data.
				GS_ASSERT_ENGINE(channel.interpolation != AnimationClipAsset::KeyframeInterpolation::Cubic);

				const AnimationTime animationTime = static_cast<AnimationTime>(fmod(currentTime, animation->duration));
				const glm::mat4 interpolatedMatrix = InterpolateBoneLocalTransform(channel.interpolation, positionSpan, rotationSpan, scaleSpan, animationTime);

				if (bone.parentBoneIndex == invalidBoneIndex) {
					boneMatrices[boneIndex] = interpolatedMatrix * boneMatrices[boneIndex];
				}
				else {
					const glm::mat4& parentMatrix = boneMatrices[bone.parentBoneIndex];
					boneMatrices[boneIndex] = parentMatrix * interpolatedMatrix * boneMatrices[boneIndex];
				}
			}

			if (animatorComponent.skeletonMatrixBuffer == nullptr) {
				Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
				Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

				Grindstone::GraphicsAPI::Buffer::CreateInfo bufferCreateInfo{
					.debugName = "Skeletal Matrix Buffer",
					.content = &boneMatrices,
					.bufferSize = sizeof(glm::mat4) * boneMatrices.size(),
					.bufferUsage =
						GraphicsAPI::BufferUsage::TransferDst |
						GraphicsAPI::BufferUsage::TransferSrc |
						GraphicsAPI::BufferUsage::Storage,
					.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU
				};
				animatorComponent.skeletonMatrixBuffer = graphicsCore->CreateBuffer(bufferCreateInfo);
			}
			else {
				animatorComponent.skeletonMatrixBuffer->UploadData(&boneMatrices);
			}
		}
	);
}
