#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"

#include "Editor/EditorManager.hpp"

#include "CodeTools.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Settings::CodeTools::Open() {
}

void Settings::CodeTools::Render() {
	ImGui::Text("Build Options");
	ImGui::Separator();

	ImGui::InputText("MsBuild Path", &msBuildPath);
}
