#include <string>
#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "SuggestedInput.hpp"

static std::string ToLowerCase(std::string data) {
	std::string outString = data;
	std::transform(
		outString.begin(),
		outString.end(),
		outString.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);

	return outString;
}

namespace Grindstone::Editor::ImguiEditor {
	size_t SuggestedInput::Render(std::vector<std::string>& unusedComponentsItems) {
		float availableWidth = ImGui::GetContentRegionAvail().x;
		ImGui::SetNextItemWidth(availableWidth);
		ImGui::InputText("##SuggestedInput", &inputString);
		if (inputString.size() > 0) {
			ImVec2 suggestionsPosition{ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
			float suggestionsWidth = ImGui::GetItemRectSize().x;
			return RenderSuggestions(
				unusedComponentsItems,
				suggestionsPosition,
				suggestionsWidth
			);
		}

		inputString = "";
		return -1;
	}
			
	size_t SuggestedInput::RenderSuggestions(
		std::vector<std::string>& unusedComponentsItems,
		ImVec2 position,
		float suggestionsWidth
	) {
		bool hasAnyAvailableOptions = false;
		size_t chosenItem = -1;

		std::string lowerCaseString = ToLowerCase(inputString);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings;

		ImGui::SetNextWindowPos(position);
		ImGui::SetNextWindowSize({ suggestionsWidth, 256.f });
		ImGui::Begin("##SuggestedInputString", nullptr, flags);
		float innerWidth = ImGui::GetContentRegionAvail().x;
		for (size_t i = 0; i < unusedComponentsItems.size(); ++i) {
			std::string fieldText = unusedComponentsItems[i];
			std::string lowerCaseFieldText = ToLowerCase(fieldText);
			if (lowerCaseFieldText.find(lowerCaseString) != -1) {
				hasAnyAvailableOptions = true;
				if (ImGui::Button(fieldText.c_str(), { innerWidth, 0 } )) {
					chosenItem = i;
				}
			}
		}

		if (!hasAnyAvailableOptions) {
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 size = ImGui::CalcTextSize("No components available");
			ImVec2 centerPos = {
				(windowSize.x - size.x) * 0.5f,
				(windowSize.y - size.y) * 0.5f
			};
			ImGui::SetCursorPos(centerPos);
			ImGui::Text("No components available");
		}

		ImGui::End();

		ImGui::PopStyleVar();

		if (chosenItem != -1) {
			inputString = "";
		}

		return chosenItem;
	}
}
