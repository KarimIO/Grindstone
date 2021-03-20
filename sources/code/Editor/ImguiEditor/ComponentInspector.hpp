#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace ECS {
		class ComponentRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ComponentInspector {
			public:
				void render(ECS::ComponentRegistrar& registrar, entt::registry& registry, entt::entity entity);
			private:
				bool isShowingPanel = true;
			};
		}
	}
}
