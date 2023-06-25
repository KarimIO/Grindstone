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
			ImGui::Text("Ready");
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
		RightAlignedText("Checking for git repo...");
		return;
	}

	if (gitRepoStatus == GitRepoStatus::NoRepo) {
		RightAlignedText("No git repo found");
		return;
	}

	if (gitRepoStatus == GitRepoStatus::RepoInitializedButUnfetched) {
		RightAlignedText("Checking git...");
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

	std::string aheadBehindText = fmt::format("Ahead: {} / Behind: {}", aheadCount, behindCount);
	// std::string changesText = fmt::format("Changes: {}", changesCount);
	std::string branchText = fmt::format("Branch: {}", gitBranchName.c_str());

	float allWidth = ImGui::GetStyle().FramePadding.x * 4.f +
		ImGui::CalcTextSize(aheadBehindText.c_str()).x +
		// ImGui::CalcTextSize(changesText.c_str()).x +
		ImGui::CalcTextSize(branchText.c_str()).x;
	AlignToRight(allWidth);

	ImGui::Text(aheadBehindText.c_str());
	// ImGui::Text(changesText.c_str());
	ImGui::Text(branchText.c_str());
}

void StatusBar::AlignToRight(float widthNeeded) {
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
}

void StatusBar::RightAlignedText(const char* text) {
	AlignToRight(ImGui::CalcTextSize(text).x + ImGui::GetStyle().FramePadding.x * 2.f);
	ImGui::Text(text);
}
