#pragma once

#include "VulkanFramebuffer.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <stdint.h>


namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanFramebuffer : public Framebuffer {
		public:
			VulkanFramebuffer(VkFramebuffer framebuffer);
			VulkanFramebuffer(Framebuffer::CreateInfo& createInfo);
			virtual ~VulkanFramebuffer() override;
		public:
			VkFramebuffer GetFramebuffer();
		public:
			virtual uint32_t GetAttachment(uint32_t attachmentIndex) override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void Clear(ClearMode mask) override;
			virtual void BindTextures(int i) override;
			virtual void Bind() override;
			virtual void BindWrite() override;
			virtual void BindRead() override;
			virtual void Unbind() override;
		private:
			VkFramebuffer framebuffer = nullptr;
		};
	}
}
