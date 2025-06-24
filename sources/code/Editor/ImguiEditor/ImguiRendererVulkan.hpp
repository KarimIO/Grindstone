#pragma once

#include <vulkan/vulkan.h>

#include "ImguiRenderer.hpp"

namespace Grindstone::GraphicsAPI {
	class WindowGraphicsBinding;
}

namespace Grindstone::GraphicsAPI::Vulkan {
	class Core;
}

namespace Grindstone::Editor::ImguiEditor {
	class ImguiRendererVulkan : public ImguiRenderer {
	public:
		ImguiRendererVulkan();
		~ImguiRendererVulkan();

		virtual GraphicsAPI::CommandBuffer* GetCommandBuffer() override;
		virtual bool PreRender() override;
		virtual void PrepareImguiRendering() override;
		virtual void PostRender() override;
		virtual void Resize() override;
		virtual ImTextureID CreateTexture(std::filesystem::path path) override;
	private:
		void WaitForResizeAndRecreateSwapchain();
		void SetupVulkanWindow(
			Grindstone::GraphicsAPI::Vulkan::Core* graphicsCore,
			Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
			int width,
			int height
		);

		void CreateOrResizeWindow(
			Grindstone::GraphicsAPI::Vulkan::Core* graphicsCore,
			Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
			int width, int height
		);

		VkDescriptorPool imguiPool = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* textureDescriptorLayout = nullptr;
		std::vector<GraphicsAPI::CommandBuffer*> commandBuffers;

		bool shouldRebuildSwapchain = false;
	};
}
