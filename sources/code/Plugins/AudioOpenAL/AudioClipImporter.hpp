#pragma once

#include <vector>
#include <map>
#include "EngineCore/Assets/AssetImporter.hpp"
#include "AudioClip.hpp"

namespace Grindstone {
	class EngineCore;

	namespace Audio {
		class AudioClipImporter : public AssetImporter {
		public:
			AudioClipImporter(EngineCore* engineCore);
			~AudioClipImporter();

			virtual void* ProcessLoadedFile(Uuid uuid) override;
			virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
			virtual void QueueReloadAsset(Uuid uuid) override;
		private:
			std::map<Uuid, AudioClipAsset> audioClips;
			EngineCore* engineCore;
		};
	}
}
