#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Preferences {
				class BasePage {
				public:
					virtual void Open() = 0;
					virtual void Render() = 0;
				};
			}
		}
	}
}
