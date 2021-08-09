#pragma once

#include <vector>
#include <string>
#include "SuggestedInput.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace Editor {
		namespace ImguiEditor {
			class NewComponentInput {
			public:
				void Render(
					ECS::Entity entity,
					std::vector<std::string>& unusedComponentsItems
				);
			private:
				SuggestedInput suggestedInput;
			};
		}
	}
}
