#pragma once

#include <Common/Graphics/CommandBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanCommandBuffer : public CommandBuffer {
		public:
			VulkanCommandBuffer(CommandBuffer::CreateInfo& createInfo);
			virtual ~VulkanCommandBuffer() override;
		public:
			void handleStep(CommandBuffer::Command* step);
			VkCommandBuffer getCommandBuffer();
		private:
			void uploadCmdBindRenderPass(CommandBindRenderPass *ci);
			void uploadCmdUnbindRenderPass(CommandUnbindRenderPass *ci);
			void uploadCmdBindDescriptorSet(CommandBindDescriptorSets *ci);
			void uploadCmdBindCommandBuffers(CommandCallCmdBuffer *ci);
			void uploadCmdBindPipeline(CommandBindPipeline *ci);
			void uploadCmdBindVertexBuffers(CommandBindVBOs *ci);
			void uploadCmdBindIndexBuffer(CommandBindIBO *ci);
			void uploadCmdDrawVertices(CommandDrawVertices *ci);
			void uploadCmdDrawIndices(CommandDrawIndices *ci);
		private:
			VkCommandBuffer command_buffer_;

			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};