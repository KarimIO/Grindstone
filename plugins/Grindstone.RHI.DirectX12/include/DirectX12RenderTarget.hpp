#pragma once

#include "../GraphicsCommon/RenderTarget.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12RenderTarget : public RenderTarget {
		public:
			//DirectX12RenderTarget(VkImage swapchain_image, VkFormat); // Build from swapchain
			DirectX12RenderTarget(RenderTargetCreateInfo ci);
			virtual ~DirectX12RenderTarget() override;
		public:
			//VkImageView getImageView();
		public:
			virtual float getAverageValue(uint32_t i) override;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) override;
		private:
			//VkImage image_;
			//VkImageView image_view_;
			//VkDeviceMemory image_memory_;
		};
	}
}
