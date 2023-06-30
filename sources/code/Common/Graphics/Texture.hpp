#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct TextureMipMapCreateInfo {
			unsigned char *data;
			uint32_t size;
			uint32_t width, height;
		};

		enum class TextureWrapMode : uint8_t {
			Repeat = 0,
			ClampToEdge,
			ClampToBorder,
			MirroredRepeat,
			MirroredClampToEdge
		};

		enum class TextureFilter : uint8_t {
			Nearest = 0,
			Linear,
			NearestMipMapNearest,
			NearestMipMapLinear,
			LinearMipMapNearest,
			LinearMipMapLinear
		};

		struct TextureOptions {
			TextureWrapMode wrapModeU = TextureWrapMode::Repeat;
			TextureWrapMode wrapModeV = TextureWrapMode::Repeat;
			TextureWrapMode wrapModeW = TextureWrapMode::Repeat;
			TextureFilter minFilter = TextureFilter::LinearMipMapLinear;
			TextureFilter magFilter = TextureFilter::Linear;
			bool shouldGenerateMipmaps = true;
		};

		class Texture {
		public:
			struct CreateInfo {
				const char* debugName;
				const char* data;
				uint32_t width, height, size;
				uint16_t mipmaps;
				bool isCubemap;
				ColorFormat format;
				TextureOptions options;
			};

			struct CubemapCreateInfo {
				unsigned char* data[6];
				uint32_t width, height;
				uint16_t mipmaps;
				ColorFormat format;
				TextureOptions options;
			};

			virtual void RecreateTexture(Texture::CreateInfo& createInfo) = 0;
		};
	};
};
