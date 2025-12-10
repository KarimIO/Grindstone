 #pragma once

#include <Common/Math.hpp>
#include <Common/Rect.hpp>

namespace Grindstone::Rendering {
	struct RenderViewData {
	public:
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 viewMatrix;
		Math::IntRect2D renderArea;

	}; // struct RenderViewData
} // namespace Grindstone::Rendering
