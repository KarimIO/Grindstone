#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class Menubar {
			public:
				void render();
			private:
				void renderFileMenu();
				void renderViewMenu();
			};
		}
	}
}
