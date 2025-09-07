#include <imgui.h>

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "UserSettingsWindow.hpp"
#include "CodeTools.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

UserSettingsWindow::UserSettingsWindow() {
	settingsTitle = "User Settings";
	pages.push_back({ "Code Tools", Grindstone::Memory::AllocatorCore::AllocateUnique<CodeTools>() });
	OpenPage(0);
}
