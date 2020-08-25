#pragma once

#include <stdint.h>
#include <Common/Graphics/DepthTarget.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanDepthTarget : public DepthTarget {
		public:
			VulkanDepthTarget(DepthTarget::CreateInfo& ci);
			virtual ~VulkanDepthTarget() override;
		public:
			VkImageView getImageView();
		public:
			virtual void BindFace(int k);
		private:
			VkImage image_;
			VkDeviceMemory  image_memory_;
			VkImageView image_view_;
		};
	};
};
