#pragma once

#include "EngineCore/ECS/Entity.hpp"

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
				void Render();
			private:
				const char* GetEntityTag(ECS::Entity entity);
				void RenderScene(SceneManagement::Scene* scene);
				void RenderEntity(ECS::Entity entity);
			private:
				bool isShowingPanel = true;
				SceneManagement::SceneManager* sceneManager;
				ImguiEditor* editor;
				ECS::Entity entityToRename;
				std::string entityRenameNewName;
			};
		}
	}
}
