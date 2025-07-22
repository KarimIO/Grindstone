 #pragma once

#include <Common/Math.hpp>

namespace Grindstone::Rendering {
	struct RenderViewData {
	public:
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 inverseViewProjectionMatrix;
		Math::Float2 renderTargetOffset;
		Math::Float2 renderTargetSize;

	}; // struct RenderViewData
} // namespace Grindstone::Rendering
