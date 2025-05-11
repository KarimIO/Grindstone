#include <iostream>
#include <GL/gl3w.h>
#include <GL/glext.h>

#include <EngineCore/Logger.hpp>

#include "GLFormats.hpp"
#include "GLImage.hpp"


using namespace Grindstone::GraphicsAPI;

// TODO: This is pretty much broken, I need to review all of this.
OpenGL::Image::Image(const Image::CreateInfo& createInfo) {
	textureType = GL_TEXTURE_CUBE_MAP;

	glGenTextures(1, &imageHandle);
	glBindTexture(textureType, imageHandle);

	if (createInfo.debugName != nullptr) {
		glObjectLabel(textureType, imageHandle, -1, createInfo.debugName);
	}

	OpenGLFormats oglFormat = TranslateFormatToOpenGL(createInfo.format);
	bool isCompressed = IsFormatCompressed(createInfo.format);
	format = oglFormat.format;
	internalFormat = oglFormat.internalFormat;
	formatType = oglFormat.type;

	const char* buffer = createInfo.initialData;
	uint8_t blockSize = GetCompressedFormatBlockSize(createInfo.format);

	gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");

	for (GLenum i = 0; i < 6; i++) {
		uint32_t width = createInfo.width;
		uint32_t height = createInfo.height;

		for (GLint j = 0; j <= createInfo.mipLevels; j++) {
			unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
			glCompressedTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				j,
				oglFormat.format,
				width,
				height,
				0,
				size,
				buffer
			);

			buffer += size;
			width /= 2;
			height /= 2;
		}
	}

	if (!isCompressed) {
		glTexSubImage2D(
			textureType,
			0, 0, 0,
			width, height,
			format, formatType,
			createInfo.initialData
		);
	}
	else {
		uint8_t blockSize = GetCompressedFormatBlockSize(createInfo.format);

		uint32_t mipWidth = createInfo.width;
		uint32_t mipHeight = createInfo.height;

		const char* buffer = createInfo.initialData;

		for (uint32_t i = 0; i <= createInfo.mipLevels; i++) {
			unsigned int size = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;
			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				i,
				oglFormat.format,
				mipWidth, mipHeight, 0,
				size,
				buffer
			);

			buffer += size;
			mipWidth >>= 1;
			mipHeight >>= 1;
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGL::Image::Resize(uint32_t width, uint32_t height) {

}

void OpenGL::Image::UploadData(const char* data, uint64_t dataSize) {
	glTexSubImage2D(
		textureType,
		0, 0, 0,
		width, height,
		format, formatType,
		data
	);
}

unsigned int OpenGL::Image::GetImage() const {
	return imageHandle;
}

void OpenGL::Image::Bind(int locationIndex) {
	if (glIsTexture(imageHandle)) {
		glBindImageTexture(
			locationIndex,
			imageHandle,
			0,
			GL_FALSE,        // layered (for array or 3D textures)
			0,               // layer (if layered = GL_TRUE)
			access,   // access: GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE
			internalFormat
		);
	}
	else {
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Invalid texture textureHandle: {}.", imageHandle);
	}
}

bool Grindstone::GraphicsAPI::OpenGL::Image::IsCubemap() const {
	return GL_TEXTURE_CUBE_MAP == textureType;
}

OpenGL::Image::~Image() {
	glDeleteTextures(1, &imageHandle);
}
