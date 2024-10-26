#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	struct TextureMipMapCreateInfo {
		unsigned char *data;
		uint32_t size;
		uint32_t width, height;
	};

	/*! TextureOptions control how textures are sampled.
		Wrap Modes dictate how an image will be sampled when trying to get data
		outside the coordinates of the image.
		Mip Filters, Min Filters, and Mag filters dictate how pixels will be sampled
		when trying to get data a fraction of the way between pixels or partway between
		mips.
	*/
	struct TextureOptions {
		TextureWrapMode wrapModeU = TextureWrapMode::Repeat;
		TextureWrapMode wrapModeV = TextureWrapMode::Repeat;
		TextureWrapMode wrapModeW = TextureWrapMode::Repeat;
		TextureFilter mipFilter = TextureFilter::Nearest;
		TextureFilter minFilter = TextureFilter::Linear;
		TextureFilter magFilter = TextureFilter::Linear;
		// Anistropy dictates how pixels are blended together as the angle to its normal
		// increase. When anisotropy == 0, it will be disabled.
		float anistropy = 0.0f;
		float mipMin = -1000.f;
		float mipMax = 1000.0f;
		float mipBias = 0.0f;
		bool shouldGenerateMipmaps = true;
	};

	/*! Textures represent image data. They can include a multitude of formats
		representing red, green, blue, and alpha channels. They represent 1d,
		2d, or 3d buffers, cubemaps, etc. These also include sampler information.
	*/
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

		virtual void RecreateTexture(const Texture::CreateInfo& createInfo) = 0;
	};
}
