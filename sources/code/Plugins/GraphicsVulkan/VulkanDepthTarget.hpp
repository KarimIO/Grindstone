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
			VkSampler GetSampler();
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void BindFace(int k);
		private:
			void CreateTextureSampler();

			VkImage image = nullptr;
			VkSampler sampler = nullptr;
			VkImageView imageView = nullptr;
			VkDeviceMemory imageMemory = nullptr;
		};
	};
};
