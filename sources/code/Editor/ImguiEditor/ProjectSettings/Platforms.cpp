#include <fstream>
#include <imgui.h>

#include <Editor/EditorManager.hpp>

#include "Platforms.hpp"
#include "PlatformWindows.hpp"
using namespace Grindstone::Editor::ImguiEditor;

Settings::Platforms::Platforms() {
	platformPages.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<PlatformWindows>());
}

void Settings::Platforms::Open() {
	for (Grindstone::UniquePtr<BasePage>& page : platformPages) {
		page->Open();
	}
}

void Settings::Platforms::Render() {
	ImGui::Text("Platforms");
	platformPages[0]->Render();
}

void Settings::Platforms::Save() {
	for (Grindstone::UniquePtr<BasePage>& page : platformPages) {
		page->Save();
	}
}

void Settings::Platforms::Reset() {
	for (Grindstone::UniquePtr<BasePage>& page : platformPages) {
		page->Reset();
	}
}
