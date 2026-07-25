#pragma once

#include <vector>
#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct RigAsset : public Asset {
		struct Bone {
			Math::Matrix4 localBindTransform;
			Math::Matrix4 inverseBindTransform;
			uint32_t parentBoneIndex;
		};

		Math::Matrix4 globalInverseTransform;
		std::vector<Bone> bones;
		std::map<std::string, uint32_t> boneNameToIndex;

		RigAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}

		DEFINE_ASSET_TYPE("Rig", AssetType::Rig)
	};
}
