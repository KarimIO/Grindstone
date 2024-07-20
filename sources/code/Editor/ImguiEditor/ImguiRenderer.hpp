#pragma once

#include <imgui.h>
#include <filesystem>

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
	}

	namespace Editor::ImguiEditor {
		class ImguiRenderer {
		public:
			static ImguiRenderer* Create();

			virtual GraphicsAPI::CommandBuffer* GetCommandBuffer() = 0;
			virtual bool PreRender() = 0;
			virtual void PrepareImguiRendering() = 0;
			virtual void PostRender() = 0;
			virtual void Resize() = 0;
			virtual ImTextureID CreateTexture(std::filesystem::path path) = 0;
		};
	}
}
