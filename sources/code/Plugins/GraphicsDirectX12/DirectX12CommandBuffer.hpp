#pragma once

#include "../GraphicsCommon/CommandBuffer.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12CommandBuffer : public CommandBuffer {
		public:
			DirectX12CommandBuffer(CommandBufferCreateInfo createInfo);
			virtual ~DirectX12CommandBuffer() override;
		public:
			ID3D12CommandList* getCommandList();
			void handleStep(CommandCreateInfo* step);
			// VkCommandBuffer getCommandBuffer();
		private:
			void uploadCmdBindRenderPass(CommandBindRenderPass* ci);
			void uploadCmdUnbindRenderPass(CommandUnbindRenderPass* ci);
			void uploadCmdBindDescriptorSet(CommandBindDescriptorSets* ci);
			void uploadCmdBindCommandBuffers(CommandCallCmdBuffer* ci);
			void uploadCmdBindGraphicsPipeline(CommandBindGraphicsPipeline* ci);
			void uploadCmdBindVertexBuffers(CommandBindVBOs* ci);
			void uploadCmdBindIndexBuffer(CommandBindIBO* ci);
			void uploadCmdDrawVertices(CommandDrawVertices* ci);
			void uploadCmdDrawIndices(CommandDrawIndices* ci);
		private:
			ID3D12GraphicsCommandList* command_list_;

			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};