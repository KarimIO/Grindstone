#pragma once

#include <filesystem>

#include <EngineCore/Assets/Loaders/AssetLoader.hpp>

namespace Grindstone::Assets {
	class FileAssetLoader : public AssetLoader {
	public:
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid) override;
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid) override;
		virtual Grindstone::Uuid GetUuidByAddress(AssetType assetType, std::string_view address) override;
	};
}
