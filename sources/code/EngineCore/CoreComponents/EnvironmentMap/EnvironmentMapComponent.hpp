#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Textures/TextureAsset.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	class WorldContextSet;

	struct EnvironmentMapComponent {
		AssetReference<TextureAsset> specularTexture;

		REFLECT("EnvironmentMap")
	};

	void SetupEnvironmentMapComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroyEnvironmentMapComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
