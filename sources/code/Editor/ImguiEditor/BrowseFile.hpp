#pragma once

#include <string>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {
		namespace ImguiEditor {
			bool BrowseFile(EngineCore* engineCore, const char* label, std::string& filepath);
		}
	}
}
