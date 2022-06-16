#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class CodeTools : public BasePage {
				public:
					virtual void Open() override;
					void WriteFile();
					virtual void Render() override;
				private:
					std::string msBuildPath;
				};
			}
		}
	}
}
