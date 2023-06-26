#pragma once

#include <string>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class StatusBar {
			public:
				StatusBar();
				void Render();
			private:
				ImTextureID GetTexture(std::string path);
				void RenderGit();
				void RenderGitWhenLoaded();
				void AlignToRight(float space);
				void RightAlignedText(const char* text);

				ImTextureID gitBranchIcon;
				ImTextureID gitAheadBehindIcon;
				ImTextureID gitChangesIcon;
			};
		}
	}
}
