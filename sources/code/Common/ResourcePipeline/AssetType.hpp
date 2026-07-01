#pragma once

#include <stdint.h>

namespace Grindstone {
	enum class AssetType : uint16_t {
		Undefined,
		Prefab,
		Texture,
		GraphicsPipelineSet,
		ComputePipelineSet,
		Material,
		AudioClip,
		Mesh3d,
		Rig,
		Animation,
		Count
	};

	const char* GetAssetTypeToString(AssetType type);
	AssetType GetAssetTypeFromString(const char* type);
}
