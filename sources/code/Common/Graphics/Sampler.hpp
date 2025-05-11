#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	/*! SamplerOptions control how textures are sampled.
		Wrap Modes dictate how an image will be sampled when trying to get data
		outside the coordinates of the image.
		Mip Filters, Min Filters, and Mag filters dictate how pixels will be sampled
		when trying to get data a fraction of the way between pixels or partway between
		mips.
	*/
	struct SamplerOptions {
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
	};

	/*! Samplers are an object that controls how textures are accessed.
	*/
	class Sampler {
	public:
		struct CreateInfo {
			const char* debugName = nullptr;
			SamplerOptions options;
		};

		virtual ~Sampler() {};
	};
}
