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
				ViewportPanel(GraphicsAPI::Core* graphicsCore);
				void render();
			private:
				bool isShowingPanel = true;
				EditorCamera* camera;
			};
		}
	}
}