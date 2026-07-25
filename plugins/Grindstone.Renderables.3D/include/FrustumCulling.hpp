#pragma once

#include <Common/Rendering/RenderViewData.hpp>

namespace Grindstone::Renderer {
	struct CullingFrustum {
		bool isOrtho; // TODO: Remove this when ortho projection CreateFrustum is fixed.
		float nearRight;
		float nearTop;
		float nearDistance;
		float farDistance;
	};

	struct AABB {
		glm::vec3 min;
		glm::vec3 max;
	};

	struct OBB {
		glm::vec3 center = {};
		glm::vec3 extents = {};
		glm::vec3 axes[3] = {};
	};

	bool IsInFrustum(const CullingFrustum& frustum, const glm::mat4& viewModelMatrix, const AABB& aabb);
	CullingFrustum CreateFrustum(const Grindstone::Rendering::RenderViewData& renderViewData);
}
