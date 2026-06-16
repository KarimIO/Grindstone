#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Textures/TextureAsset.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	class WorldContextSet;

	struct EnvironmentMapComponent {
		AssetReference<TextureAsset> specularTexture;

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		REFLECT("EnvironmentMap")
	};
}
