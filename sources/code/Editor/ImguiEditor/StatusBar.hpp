#pragma once

#include <string>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class StatusBar {
			public:
				StatusBar();
				void Render();
			private:
				void RenderGit();
			};
		}
	}
}
