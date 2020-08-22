#pragma once

#include <stdint.h>
#include "../GraphicsCommon/DepthTarget.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12DepthTarget : public DepthTarget {
		public:
			DirectX12DepthTarget(DepthTargetCreateInfo ci);
			virtual ~DirectX12DepthTarget() override;
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
