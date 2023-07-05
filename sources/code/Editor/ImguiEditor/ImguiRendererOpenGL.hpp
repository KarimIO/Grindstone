#pragma once

#include "ImguiRenderer.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiRendererOpenGL : public ImguiRenderer {
			public:
				ImguiRendererOpenGL();
				~ImguiRendererOpenGL();

				virtual void PreRender() override;
				virtual void PostRender() override;
				virtual ImTextureID CreateTexture(std::filesystem::path path) override;
			};
		}
	}
}
