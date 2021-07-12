#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;

			class SceneHeirarchyPanel {
			public:
				SceneHeirarchyPanel(SceneManagement::SceneManager* sceneManager, ImguiEditor* editor);
				void render();
			private:
				const char* getEntityTag(entt::registry& registry, entt::entity entity);
				void renderScene(SceneManagement::Scene* scene);
				void renderEntity(entt::registry& registry, entt::entity entity);
			private:
				bool isShowingPanel = true;
				SceneManagement::SceneManager* sceneManager;
				ImguiEditor* editor;
				entt::entity entityToRename = entt::null;
				std::string entityRenameNewName;
			};
		}
	}
}
