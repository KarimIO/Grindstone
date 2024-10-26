#pragma once

#include <stdint.h>
#include "../GraphicsCommon/DepthStencilTarget.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12DepthStencilTarget : public DepthStencilTarget {
		public:
			DirectX12DepthStencilTarget(DepthStencilTargetCreateInfo ci);
			virtual ~DirectX12DepthStencilTarget() override;
		public:
			// VkImageView getImageView();
		public:
			virtual void BindFace(int k);
		private:
			// VkImage image_;
			// VkDeviceMemory  image_memory_;
			// VkImageView image_view_;
		};
	};
};
