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
					void Render();
				private:
					std::vector<std::string> preprocessorDefinitions;
				};
			}
		}
	}
}
