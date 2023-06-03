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
			VkImageView GetImageView();
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void BindFace(int k);
		private:
			VkImage image = nullptr;
			VkImageView imageView = nullptr;
			VkDeviceMemory imageMemory = nullptr;
		};
	};
};
