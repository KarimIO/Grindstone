#pragma once

#include <string>
#include <imgui/imgui.h>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {
		namespace ImguiEditor {
			bool BrowseFile(EngineCore* engineCore, const char* label, std::string& filepath);
		}
	}
}
