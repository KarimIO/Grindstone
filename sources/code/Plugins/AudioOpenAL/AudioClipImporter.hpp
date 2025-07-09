#pragma once

#include <vector>
#include <map>
#include "EngineCore/Assets/AssetImporter.hpp"
#include "AudioClip.hpp"

namespace Grindstone {
	class EngineCore;

	namespace Audio {
		class AudioClipImporter : public SpecificAssetImporter<AudioClipAsset, AssetType::AudioClip> {
		public:
			virtual ~AudioClipImporter() override;

			AudioClipImporter();

			virtual void* LoadAsset(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;
		};
	}
}
