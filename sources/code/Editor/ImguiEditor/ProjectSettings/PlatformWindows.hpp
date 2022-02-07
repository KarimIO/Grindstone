#pragma once

#include <vector>
#include <string>
#include "../Settings/BaseSettingsPage.hpp"
#include "CompilerProperties.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class PlatformWindows : public BasePage {
				public:
					virtual void Open() override;
					virtual void Render() override;
				private:
					CompilerProperties compilerProperties;
				};
			}
		}
	}
}
