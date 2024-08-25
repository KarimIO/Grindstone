#pragma once

#include "ImguiRenderer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanCore;
		class WindowGraphicsBinding;
	}

	namespace Events {
		struct BaseEvent;
	}

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
				virtual void Resize() override;
				virtual ImTextureID CreateTexture(std::filesystem::path path) override;
			private:
				void WaitForResizeAndRecreateSwapchain();
				bool OnWindowResize(Events::BaseEvent* ev);
				void SetupVulkanWindow(
					Grindstone::GraphicsAPI::VulkanCore* graphicsCore,
					Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
					int width,
					int height
				);

				void CreateOrResizeWindow(
					Grindstone::GraphicsAPI::VulkanCore* graphicsCore,
					Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
					int width, int height
				);

				struct FrameData {
					GraphicsAPI::Framebuffer* framebuffer = nullptr;
					GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
				};

				VkDescriptorPool imguiPool = nullptr;
				GraphicsAPI::RenderPass* renderPass = nullptr;
				std::vector<FrameData> frameData;

				bool shouldRebuildSwapchain = false;
			};
		}
	}
}
