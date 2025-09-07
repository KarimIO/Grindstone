#pragma once

#include <Common/Graphics/Buffer.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Buffer : public Grindstone::GraphicsAPI::Buffer {
	public:
		~Buffer();
		Buffer(const Grindstone::GraphicsAPI::Buffer::CreateInfo& createInfo);
		virtual void* Map() override;
		virtual void Unmap() override;
		virtual void UploadData(const void* data, size_t size, size_t offset) override;

		GLuint GetBuffer() const;

	protected:
		GLenum bufferType = 0;
		GLuint bufferObject = 0;
	};
}
