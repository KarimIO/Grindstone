#pragma once

#include <string>
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
			uint32_t GetWidth() const;
			uint32_t GetHeight() const;

			VkImage GetImage() const;
			VkImageView GetImageView() const;
			VkSampler GetSampler() const;
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void BindFace(int k);
		private:
			void CreateTextureSampler();
			void Cleanup();
			void Create();
		private:
			VkImage image = nullptr;
			VkSampler sampler = nullptr;
			VkImageView imageView = nullptr;
			VkDeviceMemory imageMemory = nullptr;

			std::string debugName;
			DepthFormat format = DepthFormat::None;
			uint32_t width = 0;
			uint32_t height = 0;
			bool isShadowMap = false;
			bool isCubemap = false;
			bool isSampled = false;
		};
	};
};
