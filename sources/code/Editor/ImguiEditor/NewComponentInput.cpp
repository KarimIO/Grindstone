#include "EngineCore/Scenes/Scene.hpp"
#include "Editor/EditorManager.hpp"
#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void NewComponentInput::render(
				SceneManagement::Scene* scene,
				entt::entity entity,
				std::vector<std::string>& unusedComponentsItems
			) {
				size_t chosenItem = suggestedInput.render(unusedComponentsItems);
				
				if (chosenItem != -1) {
					std::string& componentToDeleteName = unusedComponentsItems[chosenItem];
					Manager::GetInstance().getCommandList().AddComponent(scene, entity, componentToDeleteName.c_str());
				}
			}
		}
	}
}
