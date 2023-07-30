#pragma once

#include <string>
#include <vector>

#include <Editor/TaskSystem.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class TaskPanel {
	public:
		void Render();
		void ToggleVisibility();
		void FetchTasks();
		std::string GetTaskButtonText();
	private:
		std::vector<Task> tasks;
		bool isShowing = false;
		size_t currentTaskCount = 0;
	};
}
