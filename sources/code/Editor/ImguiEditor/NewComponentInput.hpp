#pragma once

#include <vector>
#include <string>
#include "SuggestedInput.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace Editor {
		namespace ImguiEditor {
			class NewComponentInput {
			public:
				void render(
					SceneManagement::Scene* scene,
					entt::entity entity,
					std::vector<std::string>& unusedComponentsItems
				);
			private:
				SuggestedInput suggestedInput;
			};
		}
	}
}
