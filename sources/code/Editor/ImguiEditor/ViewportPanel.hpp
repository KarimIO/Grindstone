#pragma once

#include <Common/Event/MouseEvent.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
		class CommandBuffer;
	}

	namespace Editor {
		class EditorCamera;

		namespace ImguiEditor {
			class ViewportPanel {
			public:
				ViewportPanel();
				~ViewportPanel();
				void Render();
				void RenderCamera(GraphicsAPI::CommandBuffer* commandBuffer);
				EditorCamera* GetCamera() const;
			private:
				bool OnMouseButtonEvent(Grindstone::Events::BaseEvent* ev);
				bool OnMouseMovedEvent(Grindstone::Events::BaseEvent* ev);
				void DisplayCameraToPanel();
				void HandleInput();
				void DisplayOptions();
				void HandleSelection();
				bool isShowingPanel = true;
				bool isMovingCamera = false;
				EditorCamera* camera = nullptr;
				uint32_t width = 1;
				uint32_t height = 1;
				uint16_t renderMode = 0;

				int startDragX = 0;
				int startDragY = 0;
			};
		}
	}
}
