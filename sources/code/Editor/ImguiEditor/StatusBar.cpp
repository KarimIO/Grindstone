#include <imgui.h>
#include <imgui_internal.h>
#include "StatusBar.hpp"
#include "Editor/GitManager.hpp"
#include "Editor/EditorManager.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

StatusBar::StatusBar() {
}

void StatusBar::Render() {
	ImGuiViewport* viewport = (ImGuiViewport*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	float height = ImGui::GetFrameHeight();

	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyle().Colors[ImGuiCol_CheckMark]);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, windowFlags)) {
		if (ImGui::BeginMenuBar()) {
			RenderGit();
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void StatusBar::RenderGit() {
	GitManager& gitManager = Manager::GetInstance().GetGitManager();
	GitRepoStatus gitRepoStatus = gitManager.GetGitRepoStatus();

	if (gitRepoStatus == GitRepoStatus::NeedCheck) {
		ImGui::Text("Checking for git repo...");
		return;
	}

	if (gitRepoStatus == GitRepoStatus::NoRepo) {
		ImGui::Text("No git repo found");
		return;
	}

	if (gitRepoStatus == GitRepoStatus::RepoInitializedButUnfetched) {
		ImGui::Text("Checking git...");
		return;
	}

	RenderGitWhenLoaded();
}

void StatusBar::RenderGitWhenLoaded() {
	GitManager& gitManager = Manager::GetInstance().GetGitManager();

	std::string gitBranchName = gitManager.GetBranchName();
	uint32_t behindCount = gitManager.GetBehindCount();
	uint32_t aheadCount = gitManager.GetAheadCount();
	uint32_t changesCount = gitManager.GetChangesCount();

	ImGui::Text("Ahead: %u / Behind: %u", aheadCount, behindCount);
	ImGui::Text("Changes: %u", changesCount);
	ImGui::Text("Branch: %s", gitBranchName.c_str());
}
