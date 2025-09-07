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
