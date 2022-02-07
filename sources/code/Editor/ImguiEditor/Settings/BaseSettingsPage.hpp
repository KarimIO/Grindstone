#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class BasePage {
				public:
					virtual void Open() = 0;
					virtual void Render() = 0;
				};
			}
		}
	}
}
