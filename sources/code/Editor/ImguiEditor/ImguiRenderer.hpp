#pragma once

#include <imgui.h>
#include <filesystem>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiRenderer {
			public:
				static ImguiRenderer* Create();

				virtual void PreRender() = 0;
				virtual void PostRender() = 0;
				virtual ImTextureID CreateTexture(std::filesystem::path path) = 0;
			};
		}
	}
}
