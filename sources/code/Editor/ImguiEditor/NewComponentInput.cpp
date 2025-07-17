#include "EngineCore/Scenes/Scene.hpp"
#include "Editor/EditorManager.hpp"
#include "NewComponentInput.hpp"

namespace Grindstone::Editor::ImguiEditor {
	void NewComponentInput::Render(
		ECS::Entity entity,
		std::vector<std::string>& unusedComponentsItems
	) {
		size_t chosenItem = suggestedInput.Render(unusedComponentsItems);
				
		if (chosenItem != -1) {
			std::string& componentToDeleteName = unusedComponentsItems[chosenItem];
			auto& commandList = Manager::GetInstance().GetCommandList();
			commandList.AddComponent(entity, Grindstone::HashedString(componentToDeleteName));
		}
	}
}
