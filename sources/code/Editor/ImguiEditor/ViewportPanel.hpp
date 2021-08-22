#pragma once

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
	}

	namespace Editor {
		class EditorCamera;

		namespace ImguiEditor {
			class ViewportPanel {
			public:
				ViewportPanel();
				void Render();
			private:
				bool isShowingPanel = true;
				EditorCamera* camera;
			};
		}
	}
}
