#pragma once

#include <string>

#include "TaskPanel.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiRenderer;

			class StatusBar {
			public:
				StatusBar(ImguiRenderer* imguiRenderer);
				void Render();
			private:
				void RenderGit();
				void RenderGitWhenLoaded();
				void AlignToRight(float space);
				void RightAlignedText(const char* text);

				ImTextureID gitBranchIcon;
				ImTextureID gitAheadBehindIcon;
				ImTextureID gitChangesIcon;

				TaskPanel taskPanel;
			};
		}
	}
}
