#pragma once

#include "../GraphicsCommon/CommandBuffer.h"
#include <vector>
#include <vulkan/vulkan.h>

class vkCommandBuffer : public CommandBuffer {
	VkDevice *device;
	std::vector<VkCommandBuffer> commandBuffers;
	uint16_t m_currentBuffer;

	CommandBufferSecondaryInfo secondaryInfo;
public:
	void HandleStep(uint32_t framebufferID, VkCommandBuffer *commandBuffer, CommandCreateInfo * step);
	vkCommandBuffer(VkDevice *device, VkCommandPool *commandPool, CommandBufferCreateInfo createInfo);

	VkCommandBuffer *GetCommandBuffer(size_t id);

	void SetCommandBuffer(uint16_t currentBuffer);

	void Reset();
	void Begin();
	void End();
	void BindRenderPass(RenderPass *_renderPass, Framebuffer **framebuffers, uint32_t framebufferCount);
	void UnbindRenderPass();
	void BindCommandBuffers(CommandBuffer **cmd, uint32_t numCommands);
	void BindGraphicsPipeline(GraphicsPipeline *pipeline);
	void BindTextureDescriptor(TextureBinding *binding);
	void BindUBODescriptor(UniformBufferBinding *binding);
	void BindBufferObjects(VertexBuffer *vb, IndexBuffer *ib, bool _useLargeBuffer);
	void DrawIndexed(int32_t _baseVertex, uint32_t _indexStart, uint32_t _count, uint32_t _numInstances);
};