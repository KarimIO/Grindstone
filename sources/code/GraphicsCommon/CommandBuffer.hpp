#pragma once

#include <vector>
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "GraphicsPipeline.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		enum class CommandBufferType {
			BindVertexBuffer = 0,
			BindIndexBuffer,
			BindRenderPass,
			UnbindRenderPass,
			BindGraphicsPipeline,
			BindDescriptorSet,
			CallCommandBuffer,
			DrawVertices,
			DrawVerticesIndices
		};

		class CommandBuffer {
		public:
			virtual ~CommandBuffer() {};
		};

		struct CommandCreateInfo {
		public:
			CommandBufferType type;
		};

		struct CommandUnbindRenderPass : public CommandCreateInfo {
			CommandUnbindRenderPass() {
				type = CommandBufferType::UnbindRenderPass;
			};
		};

		struct CommandCallCmdBuffer : public CommandCreateInfo {
			CommandBuffer **commandBuffers;
			uint32_t commandBuffersCount;
			CommandCallCmdBuffer() {};
			CommandCallCmdBuffer(CommandBuffer **_commandBuffers, uint32_t _commandBuffersCount) {
				commandBuffers = _commandBuffers;
				commandBuffersCount = _commandBuffersCount;
				type = CommandBufferType::CallCommandBuffer;
			};
		};

		struct CommandBindRenderPass : public CommandCreateInfo {
			RenderPass *renderPass;
			Framebuffer *framebuffer;
			uint32_t width, height;
			ClearColorValue *colorClearValues;
			uint32_t colorClearCount;
			ClearDepthStencil depthStencilClearValue;

			CommandBindRenderPass() {};
			CommandBindRenderPass(RenderPass *_renderPass,
				Framebuffer *_framebuffer,
				uint32_t _width, 
				uint32_t _height,
				ClearColorValue *_colorClearValues,
				uint32_t _colorClearCount,
				ClearDepthStencil _depthStencilClearValue) :
					renderPass(_renderPass),
					framebuffer(_framebuffer),
					width(_width),
					height(_height),
					colorClearValues(_colorClearValues),
					colorClearCount(_colorClearCount),
					depthStencilClearValue(_depthStencilClearValue) {
				type = CommandBufferType::BindRenderPass;
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
				type = CommandBufferType::BindGraphicsPipeline;
			};
		};

		struct CommandBindDescriptorSets : public CommandCreateInfo {
			GraphicsPipeline *graphicsPipeline;
			UniformBuffer **uniformBuffers;
			uint32_t uniformBufferCount;
			TextureBinding **textureBindings;
			uint32_t textureCount;
			CommandBindDescriptorSets() {};
			CommandBindDescriptorSets(GraphicsPipeline *gp, UniformBuffer **ubs, uint32_t _uboCount, TextureBinding **_textureBindings, uint32_t _textureCount) {
				graphicsPipeline = gp;
				uniformBuffers = ubs;
				uniformBufferCount = _uboCount;
				textureBindings = _textureBindings;
				textureCount = _textureCount;
				type = CommandBufferType::BindDescriptorSet;
			};
		};

		struct CommandBindVBO : public CommandCreateInfo {
			VertexBuffer *vertexBuffer;
			CommandBindVBO() {};
			CommandBindVBO(VertexBuffer *vb) {
				vertexBuffer = vb;
				type = CommandBufferType::BindVertexBuffer;
			};
		};

		struct CommandBindIBO : public CommandCreateInfo {
			IndexBuffer  *indexBuffer;
			bool useLargeBuffer;
			CommandBindIBO() {};
			CommandBindIBO(IndexBuffer *ib, bool _useLargeBuffer) {
				indexBuffer = ib;
				useLargeBuffer = _useLargeBuffer;
				type = CommandBufferType::BindIndexBuffer;
			};
		};

		struct CommandDrawVertices : public CommandCreateInfo {
			uint32_t count;
			uint32_t numInstances;
			CommandDrawVertices() {};
			CommandDrawVertices(uint32_t _count, uint32_t _numInstances) {
				count = _count;
				numInstances = _numInstances;
				type = CommandBufferType::DrawVertices;
			};
		};

		struct CommandDrawIndices : public CommandCreateInfo {
			uint32_t indexStart;
			uint32_t count;
			uint32_t numInstances;
			int32_t baseVertex;
			CommandDrawIndices() {};
			CommandDrawIndices(int32_t _baseVertex, uint32_t _indexStart, uint32_t _count, uint32_t _numInstances) {
				indexStart = _indexStart;
				count = _count;
				numInstances = _numInstances;
				baseVertex = _baseVertex;
				type = CommandBufferType::DrawVerticesIndices;
			};
		};

		struct CommandBufferSecondaryInfo {
			bool isSecondary;
			Framebuffer *framebuffer;
			RenderPass *renderPass;
		};

		struct CommandBufferCreateInfo {
			uint32_t numOutputs = 1;
			CommandCreateInfo **steps;
			uint32_t count;
			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};