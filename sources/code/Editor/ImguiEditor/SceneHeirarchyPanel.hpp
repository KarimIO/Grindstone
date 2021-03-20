#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel {
			public:
				void render();
				unsigned int getSelectedEntity();
				void updateSelectedEntity(unsigned int);
			private:
				void renderEntity();
			private:
				bool isShowingPanel = true;
				unsigned int selectedEntity = -1;
			};
		}
	}
}
