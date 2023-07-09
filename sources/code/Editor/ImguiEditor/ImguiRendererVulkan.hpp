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

				virtual GraphicsAPI::CommandBuffer* GetCommandBuffer() override;
				virtual bool PreRender() override;
				virtual void PrepareImguiRendering() override;
				virtual void PostRender() override;
				virtual ImTextureID CreateTexture(std::filesystem::path path) override;
			private:
				VkDescriptorPool imguiPool = nullptr;
				std::vector<GraphicsAPI::CommandBuffer*> commandBuffers;
			};
		}
	}
}
