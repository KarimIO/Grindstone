#pragma once

#include <vector>
#include "EngineCore/Assets/Asset.hpp"
#include "al.h"

namespace Grindstone {
	namespace Audio {
		struct AudioClipAsset : public Asset {
			AudioClipAsset(Grindstone::Uuid uuid) : Asset(uuid, uuid.ToString()) {}

			ALuint buffer = -1;
			std::uint32_t channelCount = 0;
			std::uint32_t sampleRate = 0;
			std::uint16_t bitsPerSample = 0;

			DEFINE_ASSET_TYPE("AudioClipAsset", AssetType::AudioClip)
		};
	}
}
