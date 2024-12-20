#include "../pch.h"
#include "DirectX12DepthStencilTarget.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12Format.hpp"
#include "DirectX12Utils.hpp"
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12DepthStencilTarget::DirectX12DepthStencilTarget(DepthStencilTargetCreateInfo ci) {
			VkFormat depthFormat = TranslateDepthFormatToDirectX12(ci.format);

			createImage(ci.width, ci.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		}

		DirectX12DepthStencilTarget::~DirectX12DepthStencilTarget() {
			VkDevice device = DirectX12GraphicsWrapper::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView DirectX12DepthStencilTarget::getImageView() {
			return image_view_;
		}
		void DirectX12DepthStencilTarget::BindFace(int k) {
			std::cout << "DirectX12DepthStencilTarget::BindFace is not used.\n";
			assert(false);
		}
	}
}
