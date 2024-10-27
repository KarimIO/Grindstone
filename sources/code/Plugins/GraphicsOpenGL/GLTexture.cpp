#include <iostream>
#include <GL/gl3w.h>
#include <GL/glext.h>

#include <EngineCore/Logger.hpp>

#include "GLFormats.hpp"
#include "GLTexture.hpp"

// TODO: Setup using existing mips

using namespace Grindstone::GraphicsAPI;

static void SetupTextureProperties(GLenum textureType, const TextureOptions& createInfo) {
	if (createInfo.shouldGenerateMipmaps) {
		glGenerateMipmap(textureType);
	}

	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, TranslateWrapToOpenGL(createInfo.wrapModeU));
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, TranslateWrapToOpenGL(createInfo.wrapModeV));
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, TranslateMagFilterToOpenGL(createInfo.magFilter));

	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER,
		TranslateMinFilterToOpenGL(
			createInfo.shouldGenerateMipmaps, // TODO: Use existing mips here
			createInfo.minFilter,
			createInfo.mipFilter
		)
	);

	GLfloat maxAniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);

	float finalAniso = createInfo.anistropy > maxAniso
		? maxAniso
		: createInfo.anistropy;
	glTexParameterf(textureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, createInfo.anistropy);
	glTexParameterf(textureType, GL_TEXTURE_MIN_LOD, createInfo.mipMin);
	glTexParameterf(textureType, GL_TEXTURE_MAX_LOD, createInfo.mipMax);
	glTexParameterf(textureType, GL_TEXTURE_LOD_BIAS, createInfo.mipBias);
}

OpenGL::Texture::Texture(const Texture::CreateInfo& createInfo) {
	CreateTexture(createInfo);
}

void OpenGL::Texture::RecreateTexture(const Texture::CreateInfo& createInfo) {
	glDeleteTextures(1, &textureHandle);
	CreateTexture(createInfo);
}

void OpenGL::Texture::CreateTexture(const Texture::CreateInfo& createInfo) {
	if (createInfo.isCubemap) {
		isCubemap = true;
		glGenTextures(1, &textureHandle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
		if (createInfo.debugName != nullptr) {
			glObjectLabel(GL_TEXTURE, textureHandle, -1, createInfo.debugName);
		}

		GLint internalFormat;
		GLenum format;
		bool isCompressed;
		TranslateColorFormatToOpenGL(createInfo.format, isCompressed, format, internalFormat);

		const char *buffer = createInfo.data;
		unsigned int blockSize = (
				createInfo.format == ColorFormat::SRGB_DXT1 ||
				createInfo.format == ColorFormat::SRGB_ALPHA_DXT1 ||
				createInfo.format == ColorFormat::RGB_DXT1 ||
				createInfo.format == ColorFormat::RGBA_DXT1
			) ? 8 : 16;

		gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");

		for (GLenum i = 0; i < 6; i++) {
			uint32_t width = createInfo.width;
			uint32_t height = createInfo.height;

			for (GLint j = 0; j <= createInfo.mipmaps; j++) {
				unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
				glCompressedTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					j,
					format,
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

		SetupTextureProperties(GL_TEXTURE_CUBE_MAP, createInfo.options);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else {
		isCubemap = false;
		glGenTextures(1, &textureHandle);
		glBindTexture(GL_TEXTURE_2D, textureHandle);
		if (createInfo.debugName != nullptr) {
			glObjectLabel(GL_TEXTURE, textureHandle, -1, createInfo.debugName);
		}

		GLint internalFormat;
		GLenum format;
		bool isCompressed;
		TranslateColorFormatToOpenGL(createInfo.format, isCompressed, format, internalFormat);

		if (!isCompressed) {
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, createInfo.width, createInfo.height, 0, format, GL_UNSIGNED_BYTE, createInfo.data);
		}
		else {
			unsigned int blockSize = (createInfo.format == ColorFormat::SRGB_DXT1 || createInfo.format == ColorFormat::SRGB_ALPHA_DXT1
				|| createInfo.format == ColorFormat::RGB_DXT1 || createInfo.format == ColorFormat::RGBA_DXT1) ? 8 : 16;

			uint32_t width = createInfo.width;
			uint32_t height = createInfo.height;

			const char *buffer = createInfo.data;

			for (uint32_t i = 0; i <= createInfo.mipmaps; i++) {
				unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
				glCompressedTexImage2D(GL_TEXTURE_2D, i, format, width, height,
					0, size, buffer);

				buffer += size;
				width /= 2;
				height /= 2;
			}
		}

		SetupTextureProperties(GL_TEXTURE_2D, createInfo.options);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

unsigned int OpenGL::Texture::GetTexture() const {
	return textureHandle;
}

OpenGL::Texture::Texture(const Texture::CubemapCreateInfo& createInfo) {
	isCubemap = true;
	glGenTextures(1, &textureHandle);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);

	GLint internalFormat;
	GLenum format;
	bool isCompressed;
	TranslateColorFormatToOpenGL(createInfo.format, isCompressed, format, internalFormat);

	for (GLenum i = 0; i < 6; i++) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			internalFormat,
			createInfo.width,
			createInfo.height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			createInfo.data[i]
		);
	}

	SetupTextureProperties(GL_TEXTURE_CUBE_MAP, createInfo.options);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void OpenGL::Texture::Bind(int i) {
	if (glIsTexture(textureHandle)) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureHandle);
	}
	else {
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Invalid texture textureHandle: {}.", textureHandle);
	}
}

OpenGL::Texture::~Texture() {
	glDeleteTextures(1, &textureHandle);
}
