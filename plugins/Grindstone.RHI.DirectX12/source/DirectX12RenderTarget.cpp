#include "../pch.h"
#include "DirectX12RenderTarget.hpp"
#include "DirectX12Utils.hpp"
#include "DirectX12Format.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include <iostream>
#include <cassert>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12RenderTarget::DirectX12RenderTarget(VkImage swapchain_image, VkFormat format) : image_(swapchain_image) {
			image_view_ = createImageView(image_, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		DirectX12RenderTarget::DirectX12RenderTarget(RenderTargetCreateInfo ci) {
			uint8_t channels;
			VkFormat render_format = TranslateColorFormatToDirectX12(ci.format, channels);

			createImage(ci.width, ci.height, 1, render_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, render_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		DirectX12RenderTarget::~DirectX12RenderTarget() {
			VkDevice device = DirectX12GraphicsWrapper::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView DirectX12RenderTarget::getImageView() {
			return image_view_;
		}

		float DirectX12RenderTarget::getAverageValue(uint32_t i) {
			std::cout << "DirectX12RenderTarget::getAverageValue is not used.\n";
			assert(false);
			return 0.0f;
		}

		void DirectX12RenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
			std::cout << "DirectX12RenderTarget::RenderScreen is not used.\n";
			assert(false);
		}

	}
}