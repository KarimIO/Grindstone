#pragma once

#include <Common/Math.hpp>
#include <Common/Rect.hpp>
#include <EngineCore/Assets/Asset.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Image;
	}

	struct ProbeVolumeAsset : public Asset {
		ProbeVolumeAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}

		Grindstone::Math::Box3D bounds;
		Grindstone::Math::Float3 probeSpacing;
		Grindstone::GraphicsAPI::Image* probeData = nullptr;

		DEFINE_ASSET_TYPE("ProbeVolumeAsset", AssetType::ProbeVolume)
	};
}
