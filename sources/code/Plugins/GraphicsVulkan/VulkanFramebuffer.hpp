#pragma once

#include "VulkanFramebuffer.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <stdint.h>


namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanFramebuffer : public Framebuffer {
		public:
			VulkanFramebuffer(Framebuffer::CreateInfo& ci);
			virtual ~VulkanFramebuffer() override;
		public:
			VkFramebuffer getFramebuffer();
		public:
			virtual float getExposure(int i) override;
			virtual void Clear(ClearMode mask) override;
			virtual void CopyFrom(Framebuffer *) override;
			virtual void BindWrite(bool depth) override;
			virtual void BindTextures(int i) override;
			virtual void Bind(bool depth) override;
			virtual void BindRead() override;
			virtual void Unbind() override;
		private:
			VkFramebuffer framebuffer_;
		};
	}
}