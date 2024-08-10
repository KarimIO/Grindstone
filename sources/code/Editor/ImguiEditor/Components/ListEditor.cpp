#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "ListEditor.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Widgets::StringListEditor(std::vector<std::string>& list, std::string fieldName) {
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

void Widgets::ListEditor(
	const std::string& fieldName,
	void* list,
	size_t itemCount,
	std::function<void(void*, size_t, float)> OnRenderCallback,
	std::function<void(void*, size_t)> OnAddItemCallback,
	std::function<void(void*, size_t, size_t)> OnRemoveItemsCallback
) {
	if (ImGui::Button("+ Add Item")) {
		OnAddItemCallback(list, itemCount);
	}

	size_t itemToRemove = SIZE_MAX;
	float width = ImGui::GetContentRegionAvail().x - 50.0f;
	for (size_t i = 0; i < itemCount; ++i) {
		std::string buttonLabel = std::string("-##") + fieldName + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str())) {
			itemToRemove = i;

		}

		ImGui::SameLine();
		OnRenderCallback(list, i, width);
	}

	if (itemToRemove != SIZE_MAX) {
		OnRemoveItemsCallback(list, itemToRemove, itemToRemove);
	}
}
