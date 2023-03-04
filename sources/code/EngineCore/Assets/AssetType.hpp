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
		Count
	};
}
