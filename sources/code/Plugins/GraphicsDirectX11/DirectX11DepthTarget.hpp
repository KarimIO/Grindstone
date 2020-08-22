#pragma once

#include <stdint.h>
#include "../GraphicsCommon/DepthTarget.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11DepthTarget : public DepthTarget {
		public:
			DirectX11DepthTarget(DepthTargetCreateInfo ci);
			virtual ~DirectX11DepthTarget() override;
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
