#include "../pch.h"
#include "DirectX12RenderPass.hpp"
#include "DirectX12Framebuffer.hpp"
#include "DirectX12RenderTarget.hpp"
#include "DirectX12DepthStencilTarget.hpp"
#include "DirectX12GraphicsWrapper.hpp"

#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12Framebuffer::DirectX12Framebuffer(FramebufferCreateInfo ci) {
			uint32_t total_views = ci.num_render_target_lists;
			total_views += (ci.depth_target != nullptr) ? 1 : 0;

			VkImageView *attachments = new VkImageView[total_views];
			for (uint32_t i = 0; i < ci.num_render_target_lists; ++i) {
				attachments[i] = ((DirectX12RenderTarget *)ci.render_target_lists[i])->getImageView();
			}

			if (ci.depth_target != nullptr) {
				attachments[total_views - 1] = ((DirectX12DepthStencilTarget *)ci.depth_target)->getImageView();
			}

			DirectX12RenderPass *rp = (DirectX12RenderPass *)ci.render_pass;

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rp->getRenderPassHandle();
			framebufferInfo.attachmentCount = total_views;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = rp->getWidth();
			framebufferInfo.height = rp->getHeight();
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(DirectX12GraphicsWrapper::get().getDevice(), &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		DirectX12Framebuffer::~DirectX12Framebuffer() {
		}

		VkFramebuffer DirectX12Framebuffer::getFramebuffer() {
			return framebuffer_;
		}


		float DirectX12Framebuffer::getExposure(int i) {
			std::cout << "DirectX12Framebuffer::getExposure is not used.\n";
			assert(false);
			return 0.0f;
		}
		void DirectX12Framebuffer::Clear(ClearMode mask) {
			std::cout << "DirectX12Framebuffer::Clear is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::CopyFrom(Framebuffer *) {
			std::cout << "DirectX12Framebuffer::CopyFrom is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::BindWrite(bool depth) {
			std::cout << "DirectX12Framebuffer::BindWrite is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::BindTextures(int i) {
			std::cout << "DirectX12Framebuffer::BindTextures is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::Bind(bool depth) {
			std::cout << "DirectX12Framebuffer::Bind is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::BindRead() {
			std::cout << "DirectX12Framebuffer::BindRead is not used.\n";
			assert(false);
		}
		void DirectX12Framebuffer::Unbind() {
			std::cout << "DirectX12Framebuffer::Unbind is not used.\n";
			assert(false);
		}
	}
}
