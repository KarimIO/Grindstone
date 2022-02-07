#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "Editor/EditorManager.hpp"

#include "PlatformWindows.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Settings::PlatformWindows::Open() {
}

void Settings::PlatformWindows::Render() {
	compilerProperties.Render();
}
