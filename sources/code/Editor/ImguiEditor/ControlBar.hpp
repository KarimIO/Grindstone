#pragma once

#include "Editor/EditorManager.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiRenderer;

			class ControlBar {
			public:
				ControlBar(ImguiRenderer* imguiRenderer);
				void Render();
			private:
				bool RenderButton(ImTextureID icon, bool isSelected);
				void RenderManipulationButton(ImTextureID icon, ManipulationMode& selectedMode, ManipulationMode buttonMode);
				
				ImVec4 selectedColor;
				ImVec4 deselectedColor;
				ImVec4 selectedHighlightColor;
				ImVec4 deselectedHighlightColor;
				ImVec4 selectedActiveColor;
				ImVec4 deselectedActiveColor;

				ImTextureID pauseIcon;
				ImTextureID playIcon;
				ImTextureID translateIcon;
				ImTextureID rotateIcon;
				ImTextureID scaleIcon;
			};
		}
	}
}
