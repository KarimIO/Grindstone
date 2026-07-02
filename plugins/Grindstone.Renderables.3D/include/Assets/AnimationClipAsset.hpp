#pragma once

#include <vector>
#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct AnimationClipAsset : public Asset {
		struct BoneChannel {
			size_t parentBoneIndex;
		};

		std::vector<BoneChannel> boneChannels;

		AnimationClipAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}

		DEFINE_ASSET_TYPE("AnimationClip", AssetType::AnimationClip)
	};
}
