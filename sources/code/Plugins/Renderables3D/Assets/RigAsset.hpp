#if 0
#pragma once

#include <vector>
#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct RigAsset : public Asset {
		struct Bone {
			size_t parentBoneIndex;
			Math::Matrix4 localMatrix;
			Math::Matrix4 inverseModelMatrix;
		};

		std::vector<Bone> bones;

		DEFINE_ASSET_TYPE
	};
}
#endif
