#pragma once

#include "../GraphicsCommon/CommandBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanCommandBuffer : public CommandBuffer {
		public:
			VulkanCommandBuffer(CommandBufferCreateInfo createInfo);
			virtual ~VulkanCommandBuffer() override;
		public:
			void handleStep(CommandCreateInfo * step);
			VkCommandBuffer getCommandBuffer();
		private:
			void uploadCmdBindRenderPass(CommandBindRenderPass *ci);
			void uploadCmdUnbindRenderPass(CommandUnbindRenderPass *ci);
			void uploadCmdBindDescriptorSet(CommandBindDescriptorSets *ci);
			void uploadCmdBindCommandBuffers(CommandCallCmdBuffer *ci);
			void uploadCmdBindGraphicsPipeline(CommandBindGraphicsPipeline *ci);
			void uploadCmdBindVertexBuffer(CommandBindVBO *ci);
			void uploadCmdBindIndexBuffer(CommandBindIBO *ci);
			void uploadCmdDrawVertices(CommandDrawVertices *ci);
			void uploadCmdDrawIndices(CommandDrawIndices *ci);
		private:
			VkCommandBuffer command_buffer_;

			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};