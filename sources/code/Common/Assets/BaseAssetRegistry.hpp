#pragma once

#include <Common/Filepath.h>
#include <Common/String.h>
#include <Common/ResourcePipeline/AssetType.h>

namespace Grindstone::Assets {
	class AssetRegistry {
	public:
		struct Entry {
			Uuid uuid;
			StringRef name;
			Filepath path;
			AssetType assetType;
		};

		virtual bool HasAsset(Uuid uuid) override;
		virtual bool TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry) override;
	};
}
