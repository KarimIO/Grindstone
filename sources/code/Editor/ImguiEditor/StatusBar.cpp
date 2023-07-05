#include <imgui.h>
#include <imgui_internal.h>

#include "StatusBar.hpp"
#include "Editor/GitManager.hpp"
#include "Editor/EditorManager.hpp"
#include "ImguiRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

StatusBar::StatusBar(ImguiRenderer* imguiRenderer) {
	gitBranchIcon = imguiRenderer->CreateTexture("gitIcons/GitBranch.dds");
	gitAheadBehindIcon = imguiRenderer->CreateTexture("gitIcons/GitAheadBehind.dds");
	gitChangesIcon = imguiRenderer->CreateTexture("gitIcons/GitChanges.dds");
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

	std::string aheadBehindText = fmt::format("{} / {}", aheadCount, behindCount);
	std::string changesText = std::to_string(changesCount);

	const float iconSize = 20.0f;

	float allWidth = iconSize * 3 +
		ImGui::GetStyle().FramePadding.x * 8.f +
		ImGui::CalcTextSize(aheadBehindText.c_str()).x +
		ImGui::CalcTextSize(changesText.c_str()).x +
		ImGui::CalcTextSize(gitBranchName.c_str()).x;

	AlignToRight(allWidth);

	float cursorY = ImGui::GetCursorPosY();
	float iconY = cursorY + 2.0f;

	ImGui::SetCursorPosY(iconY);
	ImGui::Image(gitAheadBehindIcon, ImVec2(iconSize, iconSize));
	ImGui::SetCursorPosY(cursorY);
	ImGui::Text(aheadBehindText.c_str());
	ImGui::SetCursorPosY(iconY);
	ImGui::Image(gitChangesIcon, ImVec2(iconSize, iconSize));
	ImGui::SetCursorPosY(cursorY);
	ImGui::Text(changesText.c_str());
	ImGui::SetCursorPosY(iconY);
	ImGui::Image(gitBranchIcon, ImVec2(iconSize, iconSize));
	ImGui::SetCursorPosY(cursorY);
	ImGui::Text(gitBranchName.c_str());
	ImGui::SetCursorPosY(cursorY);
}

void StatusBar::AlignToRight(float widthNeeded) {
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
}

void StatusBar::RightAlignedText(const char* text) {
	AlignToRight(ImGui::CalcTextSize(text).x + ImGui::GetStyle().FramePadding.x * 2.f);
	ImGui::Text(text);
}
