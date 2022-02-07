#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class Build : public BasePage {
				public:
					virtual void Open() override;
					virtual void Render() override;
				private:
					void WriteFile();
					std::vector<std::string> sceneList;
				};
			}
		}
	}
}
