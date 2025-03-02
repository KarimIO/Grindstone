#pragma once

#include <filesystem>
#include <vector>

#include <Common/Buffer.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Assets {
	enum class AssetLoadStatus {
		Success,
		FileNotFound,
		NotEnoughMemory,
		InvalidAssetType,
		AssetNotInRegistry,
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
