#pragma once

#include <stdint.h>

#include <Common/Graphics/DLLDefs.hpp>
#include <Common/Graphics/IndexBuffer.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class IndexBuffer : public Grindstone::GraphicsAPI::IndexBuffer {
	public:
		IndexBuffer(const CreateInfo& createInfo);
		~IndexBuffer();

		void Bind();

	protected:
		GLuint buffer;

	};
}
