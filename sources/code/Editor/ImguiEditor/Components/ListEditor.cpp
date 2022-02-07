#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "ListEditor.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Components::ListEditor(std::vector<std::string>& list, std::string fieldName) {
	if (ImGui::Button("+ Add Item")) {
		list.emplace_back();
	}

	int itemToRemove = -1;
	for (auto i = 0; i < list.size(); ++i) {
		std::string buttonLabel = std::string("-##") + fieldName + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str())) {
			itemToRemove = i;
		}

		ImGui::SameLine();
		std::string label = std::string("##Field") + fieldName + std::to_string(i);
		ImGui::InputText(label.c_str(), &list[i]);
	}

	if (itemToRemove > -1) {
		list.erase(list.begin() + itemToRemove);
	}
}
