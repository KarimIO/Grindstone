#pragma once

#pragma once

#include <Common/Math.hpp>
#include <Common/Rect.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include "ProbeVolumeAsset.hpp"

namespace Grindstone {
	struct ProbeVolume {
		Grindstone::Math::Box3D bounds;
		Grindstone::Math::Float3 probeSpacing = Grindstone::Math::Float3(0.5f, 0.5f, 0.5f);
		Grindstone::AssetReference<Grindstone::ProbeVolumeAsset> probeVolumeData;

		REFLECT("ProbeVolume")
	};
}
