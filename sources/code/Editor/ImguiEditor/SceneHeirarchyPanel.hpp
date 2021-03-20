#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel {
			public:
				SceneHeirarchyPanel(SceneManagement::SceneManager* sceneManager);
				void render();
				entt::entity getSelectedEntity();
				void updateSelectedEntity(entt::entity);
			private:
				const char* getEntityTag(entt::registry& registry, entt::entity entity);
				void renderScene(SceneManagement::Scene* scene);
				void renderEntity(entt::registry& registry, entt::entity entity);
			private:
				bool isShowingPanel = true;
				entt::entity selectedEntity = entt::null;
				SceneManagement::SceneManager* sceneManager;
			};
		}
	}
}
