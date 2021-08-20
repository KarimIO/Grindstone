#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct PointLightComponent {
		Math::Float3 color;
		float attenuationRadius;
		float intensity;

		REFLECT("PointLight")
	};
}
