#pragma once

#include <filesystem>

#include <Common/Buffer.hpp>
#include <Common/Assets/ArchiveDirectory.hpp>

#include "AssetLoader.hpp"

namespace Grindstone::Assets {
	class ArchiveAssetLoader : public AssetLoader {
	public:
		ArchiveAssetLoader();
		void InitializeDirectory();
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid) override;
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid) override;
		virtual Grindstone::Uuid GetUuidByAddress(AssetType assetType, std::string_view address) override;
	protected:
		AssetLoadBinaryResult LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo);
		ArchiveDirectory archiveDirectory;

		Buffer lastBuffer;
		uint16_t lastBufferIndex = UINT16_MAX;
	};
}
