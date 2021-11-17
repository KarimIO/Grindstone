#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace ECS {
		using EntityHandle = entt::entity;
		const auto EmptyEntityHandle = entt::null;
	}
}
