#include <imgui.h>

#include "BuildPopup.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void BuildPopup::StartBuild() {
	isCompilingAssets = true;
	ImGui::OpenPopup("Compiling Assets...");
	processStatus.progress = 0.0f;
	processStatus.stageText = "Starting...";
	processStatus.detailText = "";
	buildThread = std::thread(
		BuildGame,
		&processStatus
	);
	buildThread.detach();
}

void BuildPopup::Render() {
	if (!isCompilingAssets) {
		return;
	}

	ImGui::OpenPopup("Compiling Assets...");

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f));

	if (ImGui::BeginPopupModal("Compiling Assets...", false, flags)) {
		float progress = processStatus.progress;
		ImGui::ProgressBar(progress);
		{
			std::scoped_lock scopedLock(processStatus.stringMutex);
			ImGui::Text("%s", processStatus.stageText.c_str());
			ImGui::Text("%s", processStatus.detailText.c_str());
		}
		if (progress == 1.0f) {
			if (ImGui::Button("Close")) {
				isCompilingAssets = false;
			}
		}
		ImGui::EndPopup();
	}
}
