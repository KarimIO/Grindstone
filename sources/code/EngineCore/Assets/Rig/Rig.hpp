#pragma once

#include "Common/Math.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct Rig : public Asset {
		struct Bone {
			size_t parentBoneIndex;
			Math::Matrix4 localMatrix;
			Math::Matrix4 inverseModelMatrix;
		};

		std::vector<Bone> bones;
	};
}
