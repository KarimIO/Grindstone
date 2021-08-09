#pragma once

#include <vector>
#include <string>
#include <imgui/imgui.h>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {
		namespace ImguiEditor {
			class SuggestedInput {
			public:
				size_t Render(std::vector<std::string>& unusedComponentsItems);
			private:
				size_t RenderSuggestions(
					std::vector<std::string>& unusedComponentsItems,
					ImVec2 position,
					float suggestionsWidth
				);
				std::string inputString;
			};
		}
	}
}
