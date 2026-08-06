#pragma once

#include <filesystem>
#include <vector>

#include <Common/Buffer.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Assets {
	enum class AssetLoadStatus {
		Success,				///< The asset was successfully found and not loaded.
		FileNotFound,			///< The asset could not be found.
		NotEnoughMemory,		///< The asset was found, but could not be loaded because it requires an amount of memory that could not be allocated.
		InvalidAssetType,		///< The asset was found, but has a type other than that which was attempted to be loaded.
		AssetNotInRegistry,		///< The asset could not be found in the registry, likely due to an incorrect UUID.
	};

	struct AssetLoadBinaryResult {
		AssetLoadStatus status;
		std::string displayName;
		Buffer buffer;
	};

	struct AssetLoadTextResult {
		AssetLoadStatus status;
		std::string displayName;
		std::string content;
	};

	class AssetLoader {
	public:
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid) = 0;
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid) = 0;
		virtual Grindstone::Uuid GetUuidByAddress(AssetType assetType, std::string_view address) = 0;
	};
}
