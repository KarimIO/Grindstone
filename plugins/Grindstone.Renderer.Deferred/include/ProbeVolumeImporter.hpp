#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "ProbeVolumeAsset.hpp"

namespace Grindstone {
	class ProbeVolumeImporter : public SpecificAssetImporter<ProbeVolumeAsset> {
	public:
		virtual ~ProbeVolumeImporter() override;

		virtual void* LoadAsset(Uuid uuid) override;
		virtual void QueueReloadAsset(Uuid uuid) override;

	};
}
