#pragma once

#include <vector>
#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct RigAsset : public Asset {
		struct Bone {
			Math::Matrix4 localMatrix;
			Math::Matrix4 inverseModelMatrix;
			uint16_t parentBoneIndex;
		};

		std::vector<Bone> bones;
		std::map<std::string, uint16_t> boneNameToIndex;

		RigAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}

		DEFINE_ASSET_TYPE("Rig", AssetType::Rig)
	};
}
