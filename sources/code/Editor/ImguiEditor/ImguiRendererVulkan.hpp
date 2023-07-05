#pragma once

#include "ImguiRenderer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ImguiRendererVulkan : public ImguiRenderer {
			public:
				ImguiRendererVulkan();
				~ImguiRendererVulkan();

				virtual void PreRender() override;
				virtual void PostRender() override;
				virtual ImTextureID CreateTexture(std::filesystem::path path) override;
			private:
				VkDescriptorPool imguiPool = nullptr;
				std::vector<GraphicsAPI::CommandBuffer*> commandBuffers;
			};
		}
	}
}
