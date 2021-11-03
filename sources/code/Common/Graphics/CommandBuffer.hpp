#pragma once

#include <vector>
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		enum class CommandBufferType {
			BindVertexBuffers = 0,
			BindIndexBuffer,
			BindRenderPass,
			UnbindRenderPass,
			BindPipeline,
			BindDescriptorSet,
			CallCommandBuffer,
			DrawVertices,
			DrawVerticesIndices
		};

		class CommandBuffer {
		public:
			virtual ~CommandBuffer() {};

			struct Command {
			public:
				CommandBufferType type;
			};

			struct CommandUnbindRenderPass : public Command {
				CommandUnbindRenderPass() {
					type = CommandBufferType::UnbindRenderPass;
				};
			};

			struct CommandCallCmdBuffer : public Command {
				CommandBuffer **commandBuffers = nullptr;
				uint32_t commandBuffersCount = 0;
				CommandCallCmdBuffer() {};
				CommandCallCmdBuffer(CommandBuffer **_commandBuffers, uint32_t _commandBuffersCount) {
					commandBuffers = _commandBuffers;
					commandBuffersCount = _commandBuffersCount;
					type = CommandBufferType::CallCommandBuffer;
				};
			};

			struct CommandBindRenderPass : public Command {
				RenderPass *renderPass = nullptr;
				Framebuffer *framebuffer = nullptr;
				uint32_t width = 0, height = 0;
				ClearColorValue *colorClearValues = nullptr;
				uint32_t colorClearCount = 0;
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

			struct CommandBindPipeline : public Command {
				Pipeline *graphicsPipeline = nullptr;
				CommandBindPipeline() {};
				CommandBindPipeline(Pipeline *gp) {
					graphicsPipeline = gp;
					type = CommandBufferType::BindPipeline;
				};
			};

			struct CommandBindDescriptorSets : public Command {
				Pipeline *graphicsPipeline = nullptr;
				UniformBuffer **uniformBuffers = nullptr;
				uint32_t uniformBufferCount = 0;
				TextureBinding **textureBindings = nullptr;
				uint32_t textureCount = 0;
				CommandBindDescriptorSets() {};
				CommandBindDescriptorSets(Pipeline *gp, UniformBuffer **ubs, uint32_t _uboCount, TextureBinding **_textureBindings, uint32_t _textureCount) {
					graphicsPipeline = gp;
					uniformBuffers = ubs;
					uniformBufferCount = _uboCount;
					textureBindings = _textureBindings;
					textureCount = _textureCount;
					type = CommandBufferType::BindDescriptorSet;
				};
			};

			struct CommandBindVBOs : public Command {
				VertexBuffer **vertexBuffer = nullptr;
				uint32_t vertexBufferCount = 0;
				CommandBindVBOs() {};
				CommandBindVBOs(VertexBuffer **vb, uint32_t count) {
					vertexBuffer = vb;
					vertexBufferCount = count;
					type = CommandBufferType::BindVertexBuffers;
				};
			};

			struct CommandBindIBO : public Command {
				IndexBuffer *indexBuffer = nullptr;
				bool useLargeBuffer = false;
				CommandBindIBO() {};
				CommandBindIBO(IndexBuffer *ib, bool _useLargeBuffer) {
					indexBuffer = ib;
					useLargeBuffer = _useLargeBuffer;
					type = CommandBufferType::BindIndexBuffer;
				};
			};

			struct CommandDrawVertices : public Command {
				uint32_t count = 0;
				uint32_t numInstances = 1;
				CommandDrawVertices() {};
				CommandDrawVertices(uint32_t _count, uint32_t _numInstances) {
					count = _count;
					numInstances = _numInstances;
					type = CommandBufferType::DrawVertices;
				};
			};

			struct CommandDrawIndices : public Command {
				uint32_t indexStart = 0;
				uint32_t count = 0;
				uint32_t numInstances = 1;
				int32_t baseVertex = 0;
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

			struct CreateInfo {
				uint32_t numOutputs = 1;
				Command **steps;
				uint32_t count;
				CommandBufferSecondaryInfo secondaryInfo;
			};
		};
	};
};
