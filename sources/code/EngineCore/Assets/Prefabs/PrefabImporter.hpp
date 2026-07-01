#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>

#include <Common/Buffer.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>

#include "PrefabAsset.hpp"

namespace Grindstone {
	class PrefabImporter : public SpecificAssetImporter<PrefabAsset, AssetType::Prefab> {
	public:
		virtual ~PrefabImporter() override;

		virtual void* LoadAsset(Uuid uuid) override;
		virtual void QueueReloadAsset(Uuid uuid) override;
	};
}
