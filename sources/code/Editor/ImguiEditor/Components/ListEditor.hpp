#pragma once

#include <vector>
#include <string>
#include <functional>

namespace Grindstone::Editor::ImguiEditor::Widgets {
	void StringListEditor(std::vector<std::string>& list, std::string fieldName);
	void ListEditor(
		const std::string& fieldName,
		void* list,
		size_t itemCount,
		std::function<void(void*, size_t, float)> OnRenderCallback,
		std::function<void(void*, size_t)> OnAddItemCallback,
		std::function<void(void*, size_t, size_t)> OnRemoveItemsCallback
	);
}
