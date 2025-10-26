#pragma once

#include <Windows.h>
#include <Common/Window/Window.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

#include "VulkanCore.hpp"
#include "VulkanImage.hpp"
#include <vector>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Framebuffer;
	class RenderPass;

	struct ImageSet {
		Framebuffer* framebuffer;
		Image* swapChainTarget;
		VkFence fence;
	};

	struct ImageSetNative {
		VkImage             image;
		VkImageView         imageView;
		VkFramebuffer       framebuffer;
	};

	struct WindowBindingDataNative {
		VkSwapchainKHR swapChain;
		VkRenderPass renderPass;
		uint32_t width;
		uint32_t height;
		uint32_t imageSetCount;
		VkSurfaceFormatKHR surfaceFormat;
		ImageSetNative* imageSets;
	};

	class WindowGraphicsBinding : public Grindstone::GraphicsAPI::WindowGraphicsBinding {
	public:
		~WindowGraphicsBinding();

		void CreateSwapChain();
		virtual VkSurfaceKHR GetSurface();
		virtual VkSwapchainKHR GetSwapchain();
		virtual void SubmitWindowObjects(WindowBindingDataNative& windowBindingData);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		void CreateSyncObjects();

		// Inherited via WindowGraphicsBinding
		virtual bool Initialize(Window *window) override;
		virtual bool AcquireNextImage() override;
		virtual void SubmitCommandBufferNoSynchronization(GraphicsAPI::CommandBuffer* buffer) override;
		virtual void SubmitCommandBufferForCurrentFrame(GraphicsAPI::CommandBuffer* buffer) override;
		virtual bool PresentSwapchain() override;
		virtual Grindstone::GraphicsAPI::RenderPass* GetRenderPass() override;
		virtual Grindstone::GraphicsAPI::Framebuffer* GetCurrentFramebuffer() override;
		virtual uint32_t GetCurrentImageIndex() override;
		virtual uint32_t GetMaxFramesInFlight() override;
		virtual void ImmediateSetContext() override;
		virtual void ImmediateSwapBuffers() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
	private:
		Format GetDeviceColorFormat() const;
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void CreateImageSets();
		void CreateRenderPass();
		void RecreateSwapchain();
	private:
		bool isSwapchainDirty = false;
		Window* window = nullptr;
		RenderPass* renderPass = nullptr;
		std::vector<ImageSet> imageSets;
			
		Format swapchainFormat = Format::Invalid;
		VkFormat swapchainVulkanFormat;

		VkSurfaceKHR surface = nullptr;
		VkSwapchainKHR swapChain = nullptr;

		VkExtent2D swapExtent;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		uint32_t currentFrame = 0;
		uint32_t maxFramesInFlight = 0;
		uint32_t currentSwapchainImageIndex = 0;
	};
}
