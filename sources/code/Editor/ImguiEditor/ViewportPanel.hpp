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
				void RenderCamera();
				void DisplayInGameCamera();
				void DisplayCameraToPanel(uint64_t textureID);
				void HandleInput();
				void HandleSelection();
				bool isShowingPanel = true;
				EditorCamera* camera;
			};
		}
	}
}
