#include "../pch.h"
#include "DirectX11RenderPass.hpp"
#include "DirectX11Framebuffer.hpp"
#include "DirectX11RenderTarget.hpp"
#include "DirectX11DepthTarget.hpp"
#include "DirectX11GraphicsWrapper.hpp"

#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11Framebuffer::DirectX11Framebuffer(FramebufferCreateInfo ci) {
			uint32_t total_views = ci.num_render_target_lists;
			total_views += (ci.depth_target != nullptr) ? 1 : 0;

			VkImageView *attachments = new VkImageView[total_views];
			for (uint32_t i = 0; i < ci.num_render_target_lists; ++i) {
				attachments[i] = ((DirectX11RenderTarget *)ci.render_target_lists[i])->getImageView();
			}

			if (ci.depth_target != nullptr) {
				attachments[total_views - 1] = ((DirectX11DepthTarget *)ci.depth_target)->getImageView();
			}

			DirectX11RenderPass *rp = (DirectX11RenderPass *)ci.render_pass;

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rp->getRenderPassHandle();
			framebufferInfo.attachmentCount = total_views;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = rp->getWidth();
			framebufferInfo.height = rp->getHeight();
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(DirectX11GraphicsWrapper::get().getDevice(), &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		DirectX11Framebuffer::~DirectX11Framebuffer() {
		}

		VkFramebuffer DirectX11Framebuffer::getFramebuffer() {
			return framebuffer_;
		}


		float DirectX11Framebuffer::getExposure(int i) {
			std::cout << "DirectX11Framebuffer::getExposure is not used.\n";
			assert(false);
			return 0.0f;
		}
		void DirectX11Framebuffer::Clear(ClearMode mask) {
			std::cout << "DirectX11Framebuffer::Clear is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::CopyFrom(Framebuffer *) {
			std::cout << "DirectX11Framebuffer::CopyFrom is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::BindWrite(bool depth) {
			std::cout << "DirectX11Framebuffer::BindWrite is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::BindTextures(int i) {
			std::cout << "DirectX11Framebuffer::BindTextures is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::Bind(bool depth) {
			std::cout << "DirectX11Framebuffer::Bind is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::BindRead() {
			std::cout << "DirectX11Framebuffer::BindRead is not used.\n";
			assert(false);
		}
		void DirectX11Framebuffer::Unbind() {
			std::cout << "DirectX11Framebuffer::Unbind is not used.\n";
			assert(false);
		}
	}
}
