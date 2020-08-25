#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"

#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanFramebuffer::VulkanFramebuffer(Framebuffer::CreateInfo& ci) {
			uint32_t total_views = ci.num_render_target_lists;
			total_views += (ci.depth_target != nullptr) ? 1 : 0;

			VkImageView *attachments = new VkImageView[total_views];
			for (uint32_t i = 0; i < ci.num_render_target_lists; ++i) {
				attachments[i] = ((VulkanRenderTarget *)ci.render_target_lists[i])->getImageView();
			}

			if (ci.depth_target != nullptr) {
				attachments[total_views - 1] = ((VulkanDepthTarget *)ci.depth_target)->getImageView();
			}

			VulkanRenderPass *rp = (VulkanRenderPass *)ci.render_pass;

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rp->getRenderPassHandle();
			framebufferInfo.attachmentCount = total_views;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = rp->getWidth();
			framebufferInfo.height = rp->getHeight();
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanCore::get().getDevice(), &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		VulkanFramebuffer::~VulkanFramebuffer() {
			vkDestroyFramebuffer(VulkanCore::get().getDevice(), framebuffer_, nullptr);
		}

		VkFramebuffer VulkanFramebuffer::getFramebuffer() {
			return framebuffer_;
		}


		float VulkanFramebuffer::getExposure(int i) {
			std::cout << "VulkanFramebuffer::getExposure is not used.\n";
			assert(false);
			return 0.0f;
		}
		void VulkanFramebuffer::Clear(ClearMode mask) {
			std::cout << "VulkanFramebuffer::Clear is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::CopyFrom(Framebuffer *) {
			std::cout << "VulkanFramebuffer::CopyFrom is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindWrite(bool depth) {
			std::cout << "VulkanFramebuffer::BindWrite is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindTextures(int i) {
			std::cout << "VulkanFramebuffer::BindTextures is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::Bind(bool depth) {
			std::cout << "VulkanFramebuffer::Bind is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindRead() {
			std::cout << "VulkanFramebuffer::BindRead is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::Unbind() {
			std::cout << "VulkanFramebuffer::Unbind is not used.\n";
			assert(false);
		}
	}
}
