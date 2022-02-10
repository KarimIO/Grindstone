#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			void UpdateSystem(entt::registry& registry);
			void UpdateEditorSystem(entt::registry& registry);
		}
	}
}
