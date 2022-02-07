#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "Editor/EditorManager.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"

#include "Platforms.hpp"
#include "CompilerProperties.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Settings::CompilerProperties::Render() {
	ImGui::Text("Compiler Properties");
	ImGui::Separator();
	ImGui::Text("Preprocessor Defines:");
	Components::ListEditor(preprocessorDefinitions, "PreprocessorDefines");
}
