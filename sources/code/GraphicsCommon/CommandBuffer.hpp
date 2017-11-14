#pragma once

#include <vector>
#include "Framebuffer.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"

enum CommandType {
	CMD_BIND_VERTEX_BUFFER = 0,
	CMD_BIND_INDEX_BUFFER,
	CMD_BIND_RENDER_PASS,
	CMD_UNBIND_RENDER_PASS,
	CMD_BIND_GRAPHICS_PIPELINE,
	CMD_BIND_DESCRIPTOR_SET,
	CMD_CALL_COMMAND_BUFFER,
	CMD_DRAW_VERTEX,
	CMD_DRAW_INDEX
};

class CommandBuffer {
public:

	virtual void Reset() = 0;
	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void SetCommandBuffer(uint16_t currentBuffer) = 0;

	virtual void BindRenderPass(RenderPass *renderpass, Framebuffer **framebuffers, uint32_t framebufferCount) = 0;
	virtual void UnbindRenderPass() = 0;
	virtual void BindCommandBuffers(CommandBuffer **cmd, uint32_t numCommands) = 0;
	virtual void BindGraphicsPipeline(GraphicsPipeline *pipeline) = 0;
	virtual void BindTextureDescriptor(TextureBinding *binding) = 0;
	virtual void BindUBODescriptor(UniformBufferBinding *binding) = 0;
	virtual void BindBufferObjects(VertexBuffer *vb, IndexBuffer *ib, bool _useLargeBuffer) = 0;
	virtual void DrawIndexed(int32_t _baseVertex, uint32_t _indexStart, uint32_t _count, uint32_t _numInstances) = 0;
};

struct CommandCreateInfo {
public:
	CommandType type;
};

struct CommandUnbindRenderPass : public CommandCreateInfo {
	CommandUnbindRenderPass() {
		type = CMD_UNBIND_RENDER_PASS;
	};
};

struct CommandCallCmdBuffer : public CommandCreateInfo {
	CommandBuffer **commandBuffers;
	uint32_t commandBuffersCount;
	CommandCallCmdBuffer() {};
	CommandCallCmdBuffer(CommandBuffer **_commandBuffers, uint32_t _commandBuffersCount) {
		commandBuffers = _commandBuffers;
		commandBuffersCount = _commandBuffersCount;
		type = CMD_CALL_COMMAND_BUFFER;
	};
};

struct CommandBindRenderPass : public CommandCreateInfo {
	RenderPass *renderPass;
	Framebuffer **framebuffers;
	uint32_t framebufferCount;
	uint32_t width, height;
	ClearColorValue *colorClearValues;
	uint32_t colorClearCount;
	ClearDepthStencil depthStencilClearValue;

	CommandBindRenderPass() {};
	CommandBindRenderPass(RenderPass *_renderPass) {
		renderPass = _renderPass;

		type = CMD_BIND_RENDER_PASS;
	};

	/*CommandBindRenderPass(RenderPass *_renderPass, Framebuffer ** _framebuffers, uint32_t _framebufferCount, uint32_t _width, uint32_t _height) {
		renderPass = _renderPass;
		framebuffers = _framebuffers;
		framebufferCount = _framebufferCount;
		width = _width;
		height = _height;
		colorClearValues = { {0.0f, 0.0f, 0.0f, 1.0f} };
		depthStencilClearValue = {false, 0.0f, 0};

		type = CMD_BIND_RENDER_PASS;
	};*/
};

struct CommandBindGraphicsPipeline : public CommandCreateInfo {
	GraphicsPipeline *graphicsPipeline;
	CommandBindGraphicsPipeline() {};
	CommandBindGraphicsPipeline(GraphicsPipeline *gp) {
		graphicsPipeline = gp;
		type = CMD_BIND_GRAPHICS_PIPELINE;
	};
};

struct CommandBindDescriptorSets : public CommandCreateInfo {
	GraphicsPipeline *graphicsPipeline;
	UniformBuffer **uniformBuffers;
	uint32_t uniformBufferCount;
	Texture **textures;
	uint32_t textureCount;
	CommandBindDescriptorSets() {};
	CommandBindDescriptorSets(GraphicsPipeline *gp, UniformBuffer **ubs, uint32_t _uboCount, Texture **_textures, uint32_t _textureCount) {
		graphicsPipeline = gp;
		uniformBuffers = ubs;
		uniformBufferCount = _uboCount;
		textures = _textures;
		textureCount = _textureCount;
		type = CMD_BIND_DESCRIPTOR_SET;
	};
};

struct CommandBindVBO : public CommandCreateInfo {
	VertexBuffer *vertexBuffer;
	CommandBindVBO() {};
	CommandBindVBO(VertexBuffer *vb) {
		vertexBuffer = vb;
		type = CMD_BIND_VERTEX_BUFFER;
	};
};

struct CommandBindIBO : public CommandCreateInfo {
	IndexBuffer  *indexBuffer;
	bool useLargeBuffer;
	CommandBindIBO() {};
	CommandBindIBO(IndexBuffer *ib, bool _useLargeBuffer) {
		indexBuffer = ib;
		useLargeBuffer = _useLargeBuffer;
		type = CMD_BIND_INDEX_BUFFER;
	};
};

struct CommandDrawVertices : public CommandCreateInfo {
	uint32_t count;
	uint32_t numInstances;
	CommandDrawVertices() {};
	CommandDrawVertices(uint32_t _count, uint32_t _numInstances) {
		count = _count;
		numInstances = _numInstances;
		type = CMD_DRAW_VERTEX;
	};
};

struct CommandDrawIndexed : public CommandCreateInfo {
	uint32_t indexStart;
	uint32_t count;
	uint32_t numInstances;
	int32_t baseVertex;
	CommandDrawIndexed() {};
	CommandDrawIndexed(int32_t _baseVertex, uint32_t _indexStart, uint32_t _count, uint32_t _numInstances) {
		indexStart = _indexStart;
		count = _count;
		numInstances = _numInstances;
		baseVertex = _baseVertex;
		type = CMD_DRAW_INDEX;
	};
};

struct CommandBufferSecondaryInfo {
	bool isSecondary;
	Framebuffer **fbos;
	uint32_t fboCount;
	RenderPass *renderPass;
};

struct CommandBufferCreateInfo {
	uint32_t numOutputs = 1;
	CommandCreateInfo **steps;
	uint32_t count;
	CommandBufferSecondaryInfo secondaryInfo;
};