#include <Common/Containers/Span.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>

#include <Grindstone.Renderables.3D/include/AnimationSystem.hpp>
#include <Grindstone.Renderables.3D/include/Components/AnimatorComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::Containers;

using AnimationTime = double;
using AnimationWeight = float;

const uint32_t invalidBoneIndex = std::numeric_limits<uint32_t>::max();

[[nodiscard]] static AnimationWeight GetKeyframeWeight(AnimationTime lastTimeStamp, AnimationTime nextTimeStamp, AnimationTime animationTime) {
	AnimationTime midWayLength = animationTime - lastTimeStamp;
	AnimationTime framesDiff = nextTimeStamp - lastTimeStamp;
	AnimationTime scaleFactor = midWayLength / framesDiff;
	return glm::clamp(static_cast<AnimationWeight>(scaleFactor), 0.0f, 1.0f);
}

template<typename T>
[[nodiscard]] static size_t GetKeyframeIndexByTime(const Span<const AnimationClipAsset::Keyframe<T>> keyframes, AnimationTime animationTime) {
	GS_ASSERT(keyframes.GetSize() > 0);

	if (keyframes.GetSize() == 1) {
		return 0;
	}

	if (animationTime <= keyframes[0].time) {
		return 0;
	}

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
	const Span<const AnimationClipAsset::PositionKeyframe> keyframes,
	uint32_t boneIndex,
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
	const Span<const AnimationClipAsset::RotationKeyframe> keyframes,
	uint32_t boneIndex,
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
	const Span<const AnimationClipAsset::ScaleKeyframe> keyframes,
	uint32_t boneIndex,
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
	uint32_t boneIndex,
	const Span<const AnimationClipAsset::PositionKeyframe> positionKeyframes,
	const Span<const AnimationClipAsset::RotationKeyframe> rotationKeyframes,
	const Span<const AnimationClipAsset::ScaleKeyframe> scaleframes,
	AnimationTime animationTime
) {
	switch (interpolation) {
	case AnimationClipAsset::KeyframeInterpolation::Step: {
		glm::mat4 translation = InterpolateStepPosition(positionKeyframes, boneIndex, animationTime);
		glm::mat4 rotation = InterpolateStepRotation(rotationKeyframes, boneIndex, animationTime);
		glm::mat4 scale = InterpolateStepScaling(scaleframes, boneIndex, animationTime);
		return translation * rotation * scale;
	}
	case AnimationClipAsset::KeyframeInterpolation::Linear: {
		glm::mat4 translation = InterpolateLinearPosition(positionKeyframes, animationTime);
		glm::mat4 rotation = InterpolateLinearRotation(rotationKeyframes, animationTime);
		glm::mat4 scale = InterpolateLinearScaling(scaleframes, animationTime);
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
	auto view = registry.view<Grindstone::TagComponent, Grindstone::AnimatorComponent>();
	view.each(
		[currentTime](Grindstone::TagComponent& tagComponent, Grindstone::AnimatorComponent& animatorComponent) {
			AssetReference<AnimationClipAsset> animationRef = animatorComponent.animation;
			AssetReference<RigAsset> rigAssetRef = animatorComponent.rig;
			if (!rigAssetRef.IsValid() || !animationRef.IsValid()) {
				return;
			}

			const RigAsset* rig = rigAssetRef.Get();
			AnimationClipAsset* animation = animationRef.Get();

			if (rig == nullptr || animation == nullptr) {
				return;
			}

			// Ensure all channels are sorted so that parent nodes are first
			std::sort(animation->boneChannels.begin(), animation->boneChannels.end(),
				[rig](const AnimationClipAsset::BoneChannel& a, const AnimationClipAsset::BoneChannel& b) -> bool {
					auto itA = rig->boneNameToIndex.find(a.boneName);
					auto itB = rig->boneNameToIndex.find(b.boneName);
					return itA->second < itB->second;
				}
			);

			std::vector<glm::mat4> boneMatrices;
			boneMatrices.reserve(rig->bones.size());

			for (const RigAsset::Bone& bone : rig->bones) {
				boneMatrices.push_back(bone.localBindTransform);
			}

			GS_ASSERT_ENGINE(rig->bones.size() == rig->boneNameToIndex.size());
			float ticks = animation->ticksPerSecond * currentTime;
			const AnimationTime animationTime = static_cast<AnimationTime>(fmod(ticks, animation->duration));

			uint32_t lastBoneIndex = 0;
			for (const AnimationClipAsset::BoneChannel& channel : animation->boneChannels) {
				auto boneIt = rig->boneNameToIndex.find(channel.boneName);
				if (boneIt == rig->boneNameToIndex.end()) {
					// TODO: Throttle this warning or preprocess it.
					GPRINT_WARN_V(Grindstone::LogSource::Rendering, "Bone channel for bone name \"{}\" does not exist in RigAsset \"{}\". This bone channel will be ignored.", channel.boneName, rig->name);
					continue;
				}

				size_t boneIndex = boneIt->second;
				GS_ASSERT_ENGINE_WITH_MESSAGE(boneIndex >= lastBoneIndex, "Animation channels are not in parent-before-child order for this rig.");
				GS_ASSERT_ENGINE_WITH_MESSAGE(boneIndex < rig->bones.size(), "Bone index is out of bounds for RigAsset bones vector.");
				lastBoneIndex = boneIndex;

				const RigAsset::Bone& bone = rig->bones[boneIndex];

				const AnimationClipAsset::PositionKeyframe* positionBegin = &animation->positions[channel.positionKeyOffset];
				const AnimationClipAsset::RotationKeyframe* rotationBegin = &animation->rotations[channel.rotationKeyOffset];
				const AnimationClipAsset::ScaleKeyframe* scaleBegin = &animation->scales[channel.scaleKeyOffset];

				const Span<const AnimationClipAsset::PositionKeyframe> positionSpan(positionBegin, channel.positionCount);
				const Span<const AnimationClipAsset::RotationKeyframe> rotationSpan(rotationBegin, channel.rotationCount);
				const Span<const AnimationClipAsset::ScaleKeyframe> scaleSpan(scaleBegin, channel.scaleCount);

				// This is not supported yet because Assimp does not support tangent data.
				GS_ASSERT_ENGINE(channel.interpolation != AnimationClipAsset::KeyframeInterpolation::Cubic);

				// TODO: Consider pos, rot, scale count == 0
				const glm::mat4 interpolatedMatrix = InterpolateBoneLocalTransform(channel.interpolation, boneIndex, positionSpan, rotationSpan, scaleSpan, animationTime);
				boneMatrices[boneIndex] = interpolatedMatrix;
			}

			// Move local pose to global space
			for (size_t i = 0; i < rig->bones.size(); ++i) {
				const RigAsset::Bone& bone = rig->bones[i];

				if (bone.parentBoneIndex != invalidBoneIndex) {
					const glm::mat4& parentGlobalMatrix = boneMatrices[bone.parentBoneIndex];
					const glm::mat4& localMatrix = boneMatrices[i];
					boneMatrices[i] = parentGlobalMatrix * localMatrix;
				}
			}

			size_t equalMatrices = 0;
			size_t unequalMatrices = 0;
			for (size_t i = 0; i < rig->bones.size(); ++i) {
				auto it = std::find_if(rig->boneNameToIndex.begin(), rig->boneNameToIndex.end(), [i](const std::pair<std::string, uint32_t>& a) { return a.second == i; });
				auto chanIt = std::find_if(animation->boneChannels.begin(), animation->boneChannels.end(), [it](const AnimationClipAsset::BoneChannel& a) { return a.boneName == it->first; });
				bool isAnimated = chanIt != animation->boneChannels.end();
				boneMatrices[i] = boneMatrices[i] * rig->bones[i].inverseBindTransform;
			}

			if (animatorComponent.skeletonMatrixBuffer == nullptr) {
				Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
				Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
				std::string matrixBufferName = std::format("Skinning Matrix Buffer '{}'", tagComponent.tag);

				Grindstone::GraphicsAPI::Buffer::CreateInfo bufferCreateInfo{
					.debugName = matrixBufferName.c_str(),
					.content = boneMatrices.data(), 
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
				animatorComponent.skeletonMatrixBuffer->UploadData(boneMatrices.data());
			}
		}
	);
}
