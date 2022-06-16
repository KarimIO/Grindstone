#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class CompilerProperties {
				public:
					void Open();
					void Render();
					void WriteFile();
				private:
					std::vector<std::string> preprocessorDefinitions;
				};
			}
		}
	}
}
