#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "Editor/EditorManager.hpp"

#include "Platforms.hpp"
#include "PlatformWindows.hpp"
using namespace Grindstone::Editor::ImguiEditor;

Settings::Platforms::Platforms() {
	platformPages.emplace_back(new PlatformWindows());
}

void Settings::Platforms::Open() {
}

void Settings::Platforms::Render() {
	ImGui::Text("Platforms");
	ImGui::Separator();

	platformPages[0]->Render();
}

void Settings::Platforms::WriteFile() {
}
