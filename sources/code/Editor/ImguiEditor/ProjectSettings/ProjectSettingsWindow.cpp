#include <imgui.h>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include "ProjectSettingsPage.hpp"
#include "ProjectSettingsWindow.hpp"
#include "Build.hpp"
#include "Platforms.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

ProjectSettingsWindow::ProjectSettingsWindow() {
	settingsTitle = "Project Settings";
	pages.push_back({ "Build", Grindstone::Memory::AllocatorCore::AllocateUnique<Build>() });
	pages.push_back({ "Platforms", Grindstone::Memory::AllocatorCore::AllocateUnique<Platforms>() });
	OpenPage(0);
}

void ProjectSettingsWindow::RegisterSettingsPage(std::string displayName, Grindstone::UniquePtr<Grindstone::Editor::ImguiEditor::Settings::BasePage> page) {
	pages.push_back({ displayName, std::move(page) });
}

void ProjectSettingsWindow::UnregisterSettingsPage(std::string displayName) {
	size_t index = 0;
	for (auto& page : pages) {
		if (page.title == displayName) {
			++index;
			break;
		}
	}

	pages.erase(pages.begin() + index);
}
