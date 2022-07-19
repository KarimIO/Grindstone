#pragma once

#include "Common/Math.hpp"

namespace Grindstone {
	struct Rig {
		struct Bone {
			size_t parentBoneIndex;
			Math::Matrix4 localMatrix;
			Math::Matrix4 inverseModelMatrix;
		};

		std::vector<Bone> bones;
	};
}
