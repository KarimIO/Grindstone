#pragma once

#include "Editor/EditorManager.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ControlBar {
			public:
				ControlBar();
				void Render();
			private:
				bool RenderButton(ImTextureID icon, bool isSelected);
				void RenderManipulationButton(ImTextureID icon, ManipulationMode& selectedMode, ManipulationMode buttonMode);
				ImTextureID GetTexture(std::string fileName);

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
