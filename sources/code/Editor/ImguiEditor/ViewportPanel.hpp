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
				void RenderCamera(GraphicsAPI::CommandBuffer* commandBuffer);
			private:
				void DisplayInGameCamera();
				void DisplayCameraToPanel(uint64_t textureID);
				void HandleInput();
				void HandleSelection();
				bool isShowingPanel = true;
				EditorCamera* camera = nullptr;
				uint32_t width = 1;
				uint32_t height = 1;
			};
		}
	}
}
