#include <imgui.h>

#include <Editor/EditorManager.hpp>
#include "TaskPanel.hpp"

using namespace Grindstone::Editor::ImguiEditor;

const ImVec2 WINDOW_SIZE = { 400.0f, 200.0f };
const float STATUS_COLUMN_WIDTH = 120.0f;

static const char* TaskStatusToString(Grindstone::Editor::Task::Status status) {
	static const char* strings[] = {"Queued", "In Progress", "Done"};
	return strings[static_cast<size_t>(status)];
}

void TaskPanel::Render() {
	float displaySizeY = ImGui::GetIO().DisplaySize.y;
	if (isShowing) {
		ImGui::SetNextWindowPos(ImVec2(0.0f, displaySizeY - WINDOW_SIZE.y - 24.0f));
		ImGui::SetNextWindowSize(WINDOW_SIZE);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::Begin("Job Panel", &isShowing, flags);

		if (currentTaskCount == 0) {
			const char* stringText = "No background tasks running.";
			ImVec2 textSize = ImGui::CalcTextSize(stringText);
			ImGui::SetCursorPos(ImVec2((WINDOW_SIZE.x - textSize.x) / 2.0f, (WINDOW_SIZE.y - textSize.y) / 2.0f));
			ImGui::Text(stringText);
		}
		else if (ImGui::BeginTable("TaskPanelTable", 2, ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("Job Name", 0, WINDOW_SIZE.x - STATUS_COLUMN_WIDTH);
			ImGui::TableSetupColumn("Status", 0, STATUS_COLUMN_WIDTH);

			for (size_t i = 0; i < currentTaskCount; ++i) {
				auto& task = tasks[i];
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32((i % 2 == 0) ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg));
				ImGui::TableNextRow(ImGuiTableRowFlags_None, 24.0f);
				ImGui::TableNextColumn();
				ImGui::Text("%s", task.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%s", TaskStatusToString(task.status));
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
}

void TaskPanel::FetchTasks() {
	TaskSystem& taskSystem = Editor::Manager::GetInstance().GetTaskSystem();
	tasks = taskSystem.GetTasks();
	currentTaskCount = tasks.size();
}

void TaskPanel::ToggleVisibility() {
	isShowing = !isShowing;
}

std::string TaskPanel::GetTaskButtonText() {
	std::string jobString = currentTaskCount == 0
		? "No tasks running"
		: std::to_string(currentTaskCount) + " tasks running";

	return jobString.c_str();
}
