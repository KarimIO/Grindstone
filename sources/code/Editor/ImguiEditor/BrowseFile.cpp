#include <filesystem>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "BrowseFile.hpp"
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/EngineCore.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			std::filesystem::path basePath = "../assets";
			bool BrowseFile(EngineCore* engineCore, const char* label, std::string& filepath) {
				bool hasBrowsed = false;
				auto window = engineCore->windowManager->GetWindowByIndex(0);
				ImGui::Text(label);
				std::string browseBtnLabel = std::string("Browse##") + label;
				if (ImGui::Button(browseBtnLabel.c_str())) {
					hasBrowsed = true;
					filepath = std::filesystem::relative(window->OpenFileDialogue(), basePath).string();
				}
				ImGui::SameLine();
				if (filepath.empty()) {
					ImGui::Text("No file selected.");
				}
				else {
					ImGui::Text(filepath.c_str());
				}

				return hasBrowsed;
			}
		}
	}
}
