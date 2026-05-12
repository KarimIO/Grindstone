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

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::SamplerOptions> {
		std::size_t operator()(const Grindstone::GraphicsAPI::SamplerOptions& options) const noexcept {

			size_t filters = static_cast<size_t>(options.wrapModeU) |
				static_cast<size_t>(options.wrapModeV) << 8 |
				static_cast<size_t>(options.wrapModeW) << 16 |
				static_cast<size_t>(options.mipFilter) << 24 |
				static_cast<size_t>(options.minFilter) << 32 |
				static_cast<size_t>(options.magFilter) << 40;

			size_t result = std::hash<uint32_t>{}(static_cast<uint64_t>(options.mipMin) | (static_cast<uint64_t>(options.mipMax) << 32u));
			result ^= std::hash<uint32_t>{}(static_cast<uint64_t>(options.anistropy) | (static_cast<uint64_t>(options.mipBias) << 32u));
			result ^= std::hash<size_t>{}(filters);

			return result;
		}
	};
}
