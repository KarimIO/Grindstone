#pragma once

#include <vector>
#include <GL/gl3w.h>

#include <Common/Graphics/Image.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Image : public Grindstone::GraphicsAPI::Image {
	public:
		Image(const CreateInfo& ci);
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void UploadData(const char* data, uint64_t dataSize) override;

		void Bind(int i);
		bool IsCubemap() const;

		virtual unsigned int GetImage() const;

		~Image();

	protected:
		GLsizei width, height, depth, mipCount, arrayLayerCount;
		GLuint imageHandle;
		GLenum textureType;
		GLenum format;
		GLenum internalFormat;
		GLenum formatType;
		GLenum access;
	};
}
