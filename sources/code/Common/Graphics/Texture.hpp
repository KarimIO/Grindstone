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

		struct TextureOptions {
			TextureWrapMode wrapModeU = TextureWrapMode::Repeat;
			TextureWrapMode wrapModeV = TextureWrapMode::Repeat;
			TextureWrapMode wrapModeW = TextureWrapMode::Repeat;
			TextureFilter mipFilter = TextureFilter::Nearest;
			TextureFilter minFilter = TextureFilter::Linear;
			TextureFilter magFilter = TextureFilter::Linear;
			// anisotropy == 0 implies it's off.
			float anistropy = 0.0f;
			float mipMin = -1000.f;
			float mipMax = 1000.0f;
			float mipBias = 0.0f;
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
