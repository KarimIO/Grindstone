#include <GL/gl3w.h>
#include "GLTexture.hpp"
#include <iostream>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT            0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT            0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT            0x8C4F

GLTexture::GLTexture(TextureCreateInfo ci) {
	if (ci.ddscube) {
		isCubemap = true;
		glGenTextures(1, &handle);

		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

		GLint internalFormat;
		GLenum format;
		bool is_compressed;
		TranslateColorFormats(ci.format, is_compressed, format, internalFormat);


		unsigned char *buffer = ci.data;
		unsigned int blockSize = (ci.format == FORMAT_COLOR_RGBA_DXT1) ? 8 : 16;

			gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");

		for (size_t i = 0; i < 6; i++) {
			uint32_t width = ci.width;
			uint32_t height = ci.height;

			for (uint32_t j = 0; j <= ci.mipmaps; j++) {
				unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, j, format, width, height,
					0, size, buffer);

				buffer += size;
				width /= 2;
				height /= 2;
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else {
		isCubemap = false;
		glGenTextures(1, &handle);

		glBindTexture(GL_TEXTURE_2D, handle);

		GLint internalFormat;
		GLenum format;
		bool is_compressed;
		TranslateColorFormats(ci.format, is_compressed, format, internalFormat);

		if (!is_compressed) {
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, ci.width, ci.height, 0, format, GL_UNSIGNED_BYTE, ci.data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			unsigned int blockSize = (ci.format == FORMAT_COLOR_RGBA_DXT1) ? 8 : 16;

			uint32_t width = ci.width;
			uint32_t height = ci.height;

			unsigned char *buffer = ci.data;

			gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");

			for (uint32_t i = 0; i <= ci.mipmaps; i++) {
				unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
				glCompressedTexImage2D(GL_TEXTURE_2D, i, format, width, height,
					0, size, buffer);

				buffer += size;
				width /= 2;
				height /= 2;
			}
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

GLTexture::GLTexture(CubemapCreateInfo ci) {
	isCubemap = true;
	glGenTextures(1, &handle);

	glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

	GLint internalFormat;
	GLenum format;
	bool is_compressed;
	TranslateColorFormats(ci.format, is_compressed, format, internalFormat);

	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, ci.width, ci.height, 0, format, GL_UNSIGNED_BYTE, ci.data[i]);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLTexture::Bind(int i) {
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle);
}

GLTexture::~GLTexture() {
	glDeleteTextures(1, &handle);
}

GLTextureBinding::GLTextureBinding(TextureBindingCreateInfo ci) {
	textures.reserve(ci.textureCount);
	targets.reserve(ci.textureCount);
	for (int i = 0; i < ci.textureCount; i++) {
		textures.push_back(reinterpret_cast<GLTexture *>(ci.textures[i].texture));
		targets.push_back(ci.textures[i].address);
	}
}

void GLTextureBinding::Bind() {
	for (int i = 0; i < textures.size(); i++) {
		textures[i]->Bind(targets[i]);
	}
}

GLTextureBindingLayout::GLTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) {
	subbindings = createInfo.bindings;
	subbindingCount = createInfo.bindingCount;
}

TextureSubBinding GLTextureBindingLayout::GetSubBinding(uint32_t i) {
	return subbindings[i];
}

uint32_t GLTextureBindingLayout::GetNumSubBindings() {
	return subbindingCount;
}

void TranslateColorFormats(ColorFormat inFormat, bool &is_compressed, GLenum &format, GLint &internalFormat) {

	is_compressed = false;

	switch (inFormat) {
	case FORMAT_COLOR_RGB_DXT1:
		format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		is_compressed = true;
	case FORMAT_COLOR_RGBA_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		is_compressed = true;
		break;
	case FORMAT_COLOR_RGBA_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		is_compressed = true;
		break;
	case FORMAT_COLOR_RGBA_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		is_compressed = true;
		break;
	case FORMAT_COLOR_R8:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	case FORMAT_COLOR_R8G8:
		internalFormat = GL_RG;
		format = GL_RG;
		break;
	case FORMAT_COLOR_R8G8B8:
		internalFormat = GL_RGB;
		format = GL_RGB;
		break;
	case FORMAT_COLOR_R8G8B8A8:
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
		break;
	default:
		throw std::runtime_error("Invalid Format!\n");
	}
}

void TranslateDepthFormats(DepthFormat inFormat, GLenum &format, GLint &internalFormat) {
	switch (inFormat) {
	case FORMAT_DEPTH_16:
		internalFormat = GL_DEPTH_COMPONENT16;
		format = GL_DEPTH_COMPONENT;
		break;
	case FORMAT_DEPTH_24:
		internalFormat = GL_DEPTH_COMPONENT24;
		format = GL_DEPTH_COMPONENT;
		break;
	case FORMAT_DEPTH_32:
		internalFormat = GL_DEPTH_COMPONENT32F;
		format = GL_DEPTH_COMPONENT;
		break;
	/*case FORMAT_DEPTH_16_STENCIL_8:
		internalFormat = GL_DEPTH24_STENCIL8;
		break;*/
	case FORMAT_DEPTH_24_STENCIL_8:
		internalFormat = GL_DEPTH24_STENCIL8;
		format = GL_DEPTH_COMPONENT;
		break;
	case FORMAT_DEPTH_32_STENCIL_8:
		internalFormat = GL_DEPTH32F_STENCIL8;
		format = GL_DEPTH_COMPONENT;
		break;
	/*case FORMAT_STENCIL_8:
		internalFormat = GL_DEPTH32F_STENCIL8;
		break;*/
	default:
		throw std::runtime_error("Invalid Format!\n");
	}
}
