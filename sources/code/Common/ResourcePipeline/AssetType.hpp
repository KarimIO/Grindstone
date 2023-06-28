#pragma once

#include <stdint.h>

namespace Grindstone {
	enum class AssetType : uint16_t {
		Undefined,
		Texture,
		Shader,
		Material,
		AudioClip,
		Mesh3d,
		Rig,
		Animation,
		Script,
		Scene,
		Count
	};

	const char* GetAssetTypeToString(AssetType type);
	AssetType GetAssetTypeFromString(const char* type);
}
