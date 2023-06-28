#include <string>
#include "AssetType.hpp"

const char* assetTypeStrings[] = {
	"Undefined",
	"Texture",
	"Shader",
	"Material",
	"AudioClip",
	"Mesh3d",
	"Rig",
	"Animation",
	"Script",
	"Scene"
};

const char* Grindstone::GetAssetTypeToString(AssetType type) {
	uint16_t uintType = static_cast<uint16_t>(type);
	if (uintType >= static_cast<uint16_t>(AssetType::Count)) {
		return nullptr;
	}

	return assetTypeStrings[uintType];
}

Grindstone::AssetType Grindstone::GetAssetTypeFromString(const char* type) {
	uint16_t count = static_cast<uint16_t>(AssetType::Count);
	for (uint16_t i = 1; i < count; ++i) {
		if (strcmp(type, assetTypeStrings[i]) == 0) {
			return static_cast<AssetType>(i);
		}
	}

	return AssetType::Undefined;
}
