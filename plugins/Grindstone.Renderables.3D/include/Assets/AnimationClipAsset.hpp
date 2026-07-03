#pragma once

#include <vector>
#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct AnimationClipAsset : public Asset {
		enum class KeyframeInterpolation {
			Step,
			Linear,
			Cubic
		};

		struct BoneChannel {
			std::string boneName;
			uint16_t positionCount = 0;
			uint16_t scaleCount = 0;
			uint16_t rotationCount = 0;
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

		using PositionKeyframe = Keyframe<Grindstone::Math::Float3>;
		using RotationKeyframe = Keyframe<Grindstone::Math::Float3>;
		using ScaleKeyframe = Keyframe<Grindstone::Math::Quaternion>;

		std::vector<BoneChannel> boneChannels;
		std::vector<PositionKeyframe> positions;
		std::vector<RotationKeyframe> scales;
		std::vector<ScaleKeyframe> rotations;

		double ticksPerSecond = 0.0;
		double duration = 0.0;

		AnimationClipAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}

		DEFINE_ASSET_TYPE("AnimationClip", AssetType::AnimationClip)
	};
}
