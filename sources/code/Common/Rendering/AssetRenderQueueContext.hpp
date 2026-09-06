 #pragma once

#include <entt/entt.hpp>
#include <Common/HashedString.hpp>

#include "RenderViewData.hpp"
#include "AssetRendererFilter.hpp"

namespace Grindstone::Rendering {
	struct AssetRenderQueueContext {
		const Grindstone::Rendering::RenderViewData& viewData;
		entt::registry& registry;
		Grindstone::HashedString renderQueueHash;
		AssetRendererFilter filter;
	};
} // namespace Grindstone::Rendering
