#include <string>
#include <algorithm>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "SuggestedInput.hpp"

std::string toLowerCase(std::string data) {
	std::string outString = data;
	std::transform(
		outString.begin(),
		outString.end(),
		outString.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);

	return outString;
}

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			size_t SuggestedInput::render(std::vector<std::string>& unusedComponentsItems) {
				float availableWidth = ImGui::GetContentRegionAvail().x;
				ImGui::SetNextItemWidth(availableWidth);
				ImGui::InputText("##SuggestedInput", &inputString);
				if (inputString.size() > 0) {
					ImVec2 suggestionsPosition{ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
					float suggestionsWidth = ImGui::GetItemRectSize().x;
					return renderSuggestions(
						unusedComponentsItems,
						suggestionsPosition,
						suggestionsWidth
					);
				}

				inputString = "";
				return -1;
			}
			
			size_t SuggestedInput::renderSuggestions(
				std::vector<std::string>& unusedComponentsItems,
				ImVec2 position,
				float suggestionsWidth
			) {
				size_t chosenItem = -1;

				std::string lowerCaseString = toLowerCase(inputString);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

				ImGuiWindowFlags flags =
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_HorizontalScrollbar |
					ImGuiWindowFlags_NoSavedSettings;
					//ImGuiWindowFlags_ShowBorders;


				ImGui::SetNextWindowPos(position);
				ImGui::SetNextWindowSize({ suggestionsWidth, 256.f });
				ImGui::Begin("##SuggestedInputString", nullptr, flags);
				float innerWidth = ImGui::GetContentRegionAvail().x;
				for (size_t i = 0; i < unusedComponentsItems.size(); ++i) {
					std::string fieldText = unusedComponentsItems[i];
					std::string lowerCaseFieldText = toLowerCase(fieldText);
					if (lowerCaseFieldText.find(lowerCaseString) != -1) {
						if (ImGui::Button(fieldText.c_str(), { innerWidth, 0 } )) {
							chosenItem = i;
						}
					}
				}
				ImGui::End();

				ImGui::PopStyleVar();

				if (chosenItem != -1) {
					inputString = "";
				}

				return chosenItem;
			}
		}
	}
}
