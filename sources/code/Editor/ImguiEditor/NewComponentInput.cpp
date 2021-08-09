#include "EngineCore/Scenes/Scene.hpp"
#include "Editor/EditorManager.hpp"
#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void NewComponentInput::Render(
				ECS::Entity entity,
				std::vector<std::string>& unusedComponentsItems
			) {
				size_t chosenItem = suggestedInput.Render(unusedComponentsItems);
				
				if (chosenItem != -1) {
					std::string& componentToDeleteName = unusedComponentsItems[chosenItem];
					auto& commandList = Manager::GetInstance().getCommandList();
					commandList.AddComponent(entity, componentToDeleteName.c_str());
				}
			}
		}
	}
}
