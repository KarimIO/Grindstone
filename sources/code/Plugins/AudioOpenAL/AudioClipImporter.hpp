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

			AudioClipImporter(EngineCore* engineCore);

			virtual void* ProcessLoadedFile(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;
		private:
			EngineCore* engineCore;
		};
	}
}
