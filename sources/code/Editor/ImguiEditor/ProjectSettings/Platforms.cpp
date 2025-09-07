#include <fstream>
#include <imgui.h>

#include <Editor/EditorManager.hpp>

#include "Platforms.hpp"
#include "PlatformWindows.hpp"
using namespace Grindstone::Editor::ImguiEditor;

Settings::Platforms::Platforms() {
	platformPages.emplace_back(Grindstone::Memory::AllocatorCore::Allocate<PlatformWindows>());
}

Settings::Platforms::~Platforms() {
	for (Grindstone::Editor::ImguiEditor::Settings::BasePage* page : platformPages) {
		Grindstone::Memory::AllocatorCore::Free(page);
	}
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
