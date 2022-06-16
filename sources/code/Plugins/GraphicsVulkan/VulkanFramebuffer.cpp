#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"

#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanFramebuffer::VulkanFramebuffer(Framebuffer::CreateInfo& ci) {
			uint32_t total_views = ci.numRenderTargetLists;
			total_views += (ci.depthTarget != nullptr) ? 1 : 0;

			VkImageView *attachments = new VkImageView[total_views];
			for (uint32_t i = 0; i < ci.numRenderTargetLists; ++i) {
				attachments[i] = ((VulkanRenderTarget *)ci.renderTargetLists[i])->GetImageView();
			}

			if (ci.depthTarget != nullptr) {
				attachments[total_views - 1] = ((VulkanDepthTarget *)ci.depthTarget)->GetImageView();
			}

			VulkanRenderPass *rp = (VulkanRenderPass *)ci.renderPass;

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rp->GetRenderPassHandle();
			framebufferInfo.attachmentCount = total_views;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = rp->GetWidth();
			framebufferInfo.height = rp->GetHeight();
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanCore::Get().GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		VulkanFramebuffer::~VulkanFramebuffer() {
			vkDestroyFramebuffer(VulkanCore::Get().GetDevice(), framebuffer, nullptr);
		}

		VkFramebuffer VulkanFramebuffer::GetFramebuffer() {
			return framebuffer;
		}


		uint32_t VulkanFramebuffer::GetAttachment(uint32_t attachmentIndex) {
			std::cout << "VulkanFramebuffer::GetAttachment is not used.\n";
			assert(false);
			return 0.0f;
		}

		void VulkanFramebuffer::Resize(uint32_t width, uint32_t height) {
			std::cout << "VulkanFramebuffer::Resize is not used.\n";
			assert(false);
		}

		void VulkanFramebuffer::Clear(ClearMode mask) {
			std::cout << "VulkanFramebuffer::Clear is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::Bind() {
			std::cout << "VulkanFramebuffer::Bind is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindWrite() {
			std::cout << "VulkanFramebuffer::BindWrite is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindRead() {
			std::cout << "VulkanFramebuffer::BindRead is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::BindTextures(int i) {
			std::cout << "VulkanFramebuffer::BindTextures is not used.\n";
			assert(false);
		}
		void VulkanFramebuffer::Unbind() {
			std::cout << "VulkanFramebuffer::Unbind is not used.\n";
			assert(false);
		}
	}
}
