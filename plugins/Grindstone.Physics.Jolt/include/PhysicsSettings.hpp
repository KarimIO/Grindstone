#pragma once

#include <glm/glm.hpp>

#include <Common/PhysicsLayer.hpp>
#include <Common/String.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>

namespace Grindstone::Physics {
	struct PhysicsSettings {
		glm::vec3 gravity;
		Grindstone::Physics::LayerMask layerMasks[Grindstone::Physics::MaxLayerCount];
		Grindstone::String layerNames[Grindstone::Physics::MaxLayerCount];
		uint8_t layerCount = 0;

		REFLECT("Grindstone::Physics::PhysicsSettings")
	};
}
