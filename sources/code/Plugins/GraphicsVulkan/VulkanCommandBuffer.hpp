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
			void HandleStep(CommandBuffer::Command* step);
			VkCommandBuffer GetCommandBuffer();
		private:
			void UploadCmdBindRenderPass(CommandBindRenderPass *ci);
			void UploadCmdUnbindRenderPass(CommandUnbindRenderPass *ci);
			void UploadCmdBindDescriptorSet(CommandBindDescriptorSets *ci);
			void UploadCmdBindCommandBuffers(CommandCallCmdBuffer *ci);
			void UploadCmdBindPipeline(CommandBindPipeline *ci);
			void UploadCmdBindVertexBuffers(CommandBindVBOs *ci);
			void UploadCmdBindIndexBuffer(CommandBindIBO *ci);
			void UploadCmdDrawVertices(CommandDrawVertices *ci);
			void UploadCmdDrawIndices(CommandDrawIndices *ci);
		private:
			VkCommandBuffer commandBuffer;

			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};
