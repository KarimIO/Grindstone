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

namespace Grindstone {
	namespace GraphicsAPI {
		GLTexture::GLTexture(CreateInfo& ci) {
			if (ci.isCubemap) {
				is_cubemap_ = true;
				glGenTextures(1, &handle_);

				glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);

				GLint internalFormat;
				GLenum format;
				bool is_compressed;
				translateColorFormats(ci.format, is_compressed, format, internalFormat);


				const char *buffer = ci.data;
				unsigned int blockSize = (ci.format == ColorFormat::SRGB_DXT1 || ci.format == ColorFormat::SRGB_ALPHA_DXT1
					|| ci.format == ColorFormat::RGB_DXT1 || ci.format == ColorFormat::RGBA_DXT1) ? 8 : 16;

				gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");

				for (size_t i = 0; i < 6; i++) {
					uint32_t width = ci.width;
					uint32_t height = ci.height;

					for (uint32_t j = 0; j <= ci.mipmaps; j++) {
						unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
						glCompressedTexImage2D(
							(GLenum) GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
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

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, translateTexWrap(ci.options.wrapModeU));
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, translateTexWrap(ci.options.wrapModeV));
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, translateTexWrap(ci.options.wrapModeW));
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, translateTexFilter(ci.options.magFilter));
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, translateTexFilter(ci.options.minFilter));

				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
			else {
				is_cubemap_ = false;
				glGenTextures(1, &handle_);

				glBindTexture(GL_TEXTURE_2D, handle_);

				GLint internalFormat;
				GLenum format;
				bool is_compressed;
				translateColorFormats(ci.format, is_compressed, format, internalFormat);

				if (!is_compressed) {
					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, ci.width, ci.height, 0, format, GL_UNSIGNED_BYTE, ci.data);

					if (ci.options.shouldGenerateMipmaps)
						glGenerateMipmap(GL_TEXTURE_2D);
				}
				else {
					unsigned int blockSize = (ci.format == ColorFormat::SRGB_DXT1 || ci.format == ColorFormat::SRGB_ALPHA_DXT1
						|| ci.format == ColorFormat::RGB_DXT1 || ci.format == ColorFormat::RGBA_DXT1) ? 8 : 16;

					uint32_t width = ci.width;
					uint32_t height = ci.height;

					const char *buffer = ci.data;

					gl3wGetProcAddress("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT");
					gl3wGetProcAddress("GL_EXT_texture_sRGB");

					for (uint32_t i = 0; i <= ci.mipmaps; i++) {
						unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
						glCompressedTexImage2D(GL_TEXTURE_2D, i, format, width, height,
							0, size, buffer);

						buffer += size;
						width /= 2;
						height /= 2;
					}
				}

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, translateTexWrap(ci.options.wrapModeU));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, translateTexWrap(ci.options.wrapModeV));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translateTexFilter(ci.options.magFilter));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translateTexFilter(ci.options.minFilter));

				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		unsigned int GLTexture::getTexture() {
			return handle_;
		}

		GLenum GLTexture::translateTexWrap(TextureWrapMode m) {
			switch (m) {
			default:
				printf("Invalid Texture Wrap Mode!\r\n");
			case TextureWrapMode::Repeat:
				return GL_REPEAT;

			case TextureWrapMode::ClampToEdge:
				return GL_CLAMP_TO_EDGE;

			case TextureWrapMode::ClampToBorder:
				return GL_CLAMP_TO_BORDER;

			case TextureWrapMode::MirroredRepeat:
				return GL_MIRRORED_REPEAT;

			case TextureWrapMode::MirroredClampToEdge:
				return GL_MIRROR_CLAMP_TO_EDGE;
			}
		}

		GLenum GLTexture::translateTexFilter(TextureFilter m) {
			switch (m) {
			default:
				printf("Invalid Filter!\r\n");
			case TextureFilter::Nearest:
				return GL_NEAREST;

			case TextureFilter::Linear:
				return GL_LINEAR;

			case TextureFilter::NearestMipMapNearest:
				return GL_NEAREST_MIPMAP_NEAREST;

			case TextureFilter::LinearMipMapNearest:
				return GL_LINEAR_MIPMAP_NEAREST;

			case TextureFilter::NearestMipMapLinear:
				return GL_NEAREST_MIPMAP_LINEAR;

			case TextureFilter::LinearMipMapLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			}
		}

		GLTexture::GLTexture(CubemapCreateInfo& ci) {
			is_cubemap_ = true;
			glGenTextures(1, &handle_);

			glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);

			GLint internalFormat;
			GLenum format;
			bool is_compressed;
			translateColorFormats(ci.format, is_compressed, format, internalFormat);

			for (size_t i = 0; i < 6; i++) {
				glTexImage2D(
					(GLenum) GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					internalFormat,
					ci.width,
					ci.height,
					0,
					format,
					GL_UNSIGNED_BYTE,
					ci.data[i]
				);
			}

			if (ci.options.shouldGenerateMipmaps)
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, translateTexWrap(ci.options.wrapModeU));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, translateTexWrap(ci.options.wrapModeV));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, translateTexWrap(ci.options.wrapModeW));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, translateTexFilter(ci.options.magFilter));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, translateTexFilter(ci.options.minFilter));

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}

		void GLTexture::bind(int i) {
			if (glIsTexture(handle_)) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(is_cubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle_);
			}
			else {
				std::cout << "Invalid texture handle_: " << handle_ << std::endl;
			}
		}

		GLTexture::~GLTexture() {
			glDeleteTextures(1, &handle_);
		}

		GLTextureBinding::GLTextureBinding(CreateInfo& ci) {
			textures_.reserve(ci.textureCount);
			targets_.reserve(ci.textureCount);
			for (uint32_t i = 0; i < ci.textureCount; i++) {
				GLTexture *t = (GLTexture *)(ci.textures[i].texture);
				if (t) {
					textures_.push_back(t);
					targets_.push_back(ci.textures[i].address);
				}
			}
		}

		void GLTextureBinding::bind() {
			for (int i = 0; i < textures_.size(); i++) {
				if (textures_[i])
					textures_[i]->bind(targets_[i]);
			}
		}

		GLTextureBindingLayout::GLTextureBindingLayout(CreateInfo& createInfo) {
			subbindings_ = createInfo.bindings;
			subbinding_count_ = createInfo.bindingCount;
		}

		TextureSubBinding GLTextureBindingLayout::getSubBinding(uint32_t i) {
			return subbindings_[i];
		}

		uint32_t GLTextureBindingLayout::getNumSubBindings() {
			return subbinding_count_;
		}

		void translateColorFormats(ColorFormat inFormat, bool &is_compressed, GLenum &format, GLint &internalFormat) {

			is_compressed = false;

			switch (inFormat) {
			case ColorFormat::RGB_DXT1:
				format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				is_compressed = true;
				break;
			case ColorFormat::RGBA_DXT1:
				format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				is_compressed = true;
				break;
			case ColorFormat::RGBA_DXT3:
				format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				is_compressed = true;
				break;
			case ColorFormat::RGBA_DXT5:
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				is_compressed = true;
				break;
			case ColorFormat::R10G10B10A2:
				internalFormat = GL_RGB10_A2;
				format = GL_RGBA;
				break;
			case ColorFormat::R8:
				internalFormat = GL_R8;
				format = GL_RED;
				break;
			case ColorFormat::R8G8:
				internalFormat = GL_RG8;
				format = GL_RG;
				break;
			case ColorFormat::R8G8B8:
				internalFormat = GL_RGB8;
				format = GL_RGB;
				break;
			case ColorFormat::R8G8B8A8:
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				break;
			case ColorFormat::R16:
				internalFormat = GL_R16F;
				format = GL_RED;
				break;
			case ColorFormat::R16G16:
				internalFormat = GL_RG16F;
				format = GL_RG;
				break;
			case ColorFormat::R16G16B16:
				internalFormat = GL_RGB16F;
				format = GL_RGB;
				break;
			case ColorFormat::R16G16B16A16:
				internalFormat = GL_RGBA16F;
				format = GL_RGBA;
				break;
			case ColorFormat::R32G32B32:
				internalFormat = GL_RGB32F;
				format = GL_RGBA;
				break;
			case ColorFormat::R32G32B32A32:
				internalFormat = GL_RGBA32F;
				format = GL_RGBA;
				break;
			default:
				throw std::runtime_error("Invalid Format!\n");
			}
		}

		void translateDepthFormats(DepthFormat inFormat, GLenum &format, GLint &internalFormat) {
			switch (inFormat) {
			case DepthFormat::D16:
				internalFormat = GL_DEPTH_COMPONENT16;
				format = GL_DEPTH_COMPONENT;
				break;
			case DepthFormat::D24:
				internalFormat = GL_DEPTH_COMPONENT24;
				format = GL_DEPTH_COMPONENT;
				break;
			case DepthFormat::D32:
				internalFormat = GL_DEPTH_COMPONENT32F;
				format = GL_DEPTH_COMPONENT;
				break;
				/*case DepthFormat::D16_STENCIL_8:
					internalFormat = GL_DEPTH24_STENCIL8;
					break;*/
			case DepthFormat::D24_STENCIL_8:
				internalFormat = GL_DEPTH24_STENCIL8;
				format = GL_DEPTH_COMPONENT;
				break;
			case DepthFormat::D32_STENCIL_8:
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
	}
}