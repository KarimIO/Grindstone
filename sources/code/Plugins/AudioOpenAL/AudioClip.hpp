#pragma once

#include <vector>
#include "EngineCore/Assets/Asset.hpp"
#include "al.h"

namespace Grindstone {
	namespace Audio {
		struct AudioClipAsset : public Asset {
			AudioClipAsset(
				Uuid uuid,
				std::string_view name,
				ALuint buffer,
				std::uint32_t channelCount,
				std::uint32_t sampleRate,
				std::uint8_t bitsPerSample
			) : Asset(uuid, name),
				buffer(buffer),
				channelCount(channelCount),
				sampleRate(sampleRate),
				bitsPerSample(bitsPerSample) {}

			ALuint buffer = -1;
			std::uint32_t channelCount = 0;
			std::uint32_t sampleRate = 0;
			std::uint8_t bitsPerSample = 0;

			DEFINE_ASSET_TYPE("AudioClipAsset", AssetType::AudioClip)
		};
	}
}
