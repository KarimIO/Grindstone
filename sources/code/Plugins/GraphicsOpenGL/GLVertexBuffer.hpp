#pragma once

#include <stdint.h>

#include <Common/Graphics/VertexBuffer.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class VertexBuffer : public Grindstone::GraphicsAPI::VertexBuffer {
	public:
		VertexBuffer(const CreateInfo& createInfo);
		~VertexBuffer();

		const VertexBufferLayout &GetLayout() const;
		virtual void UpdateBuffer(void *content);
		void Bind();

	protected:
		uint32_t size;
		GLuint vertexBuffer;
		VertexBufferLayout vertexLayout;

	};
}
