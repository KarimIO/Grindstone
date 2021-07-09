#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void NewComponentInput::render(
				entt::registry& registry,
				entt::entity entity,
				std::vector<std::string>& unusedComponentsItems,
				std::vector<ECS::ComponentFunctions>& unusedComponentsFunctions
			) {
				size_t chosenItem = suggestedInput.render(unusedComponentsItems);
				
				if (chosenItem != -1) {
					unusedComponentsFunctions[chosenItem].createComponentFn(registry, entity);
				}
			}
		}
	}
}
