 #pragma once

#include <entt/entt.hpp>
#include <Common/Containers/Span.hpp>

namespace Grindstone::Rendering {
	// Manage which entities are to be rendered by RenderQueue.
	struct AssetRendererFilter {
		Grindstone::Containers::Span<entt::entity> whitelist;
		Grindstone::Containers::Span<entt::entity> blacklist;
		bool useWhitelist = false;
	};
} // namespace Grindstone::Rendering
