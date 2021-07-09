#pragma once

#include <vector>
#include <string>
#include "SuggestedInput.hpp"
#include "EngineCore/ECS/ComponentFunctions.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class NewComponentInput {
			public:
				void render(
					entt::registry& registry,
					entt::entity entity,
					std::vector<std::string>& unusedComponentsItems,
					std::vector<ECS::ComponentFunctions>& unusedComponentsFunctions
				);
			private:
				SuggestedInput suggestedInput;
			};
		}
	}
}
