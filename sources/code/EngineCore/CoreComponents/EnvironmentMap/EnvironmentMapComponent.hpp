#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Textures/TextureAsset.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	struct EnvironmentMapComponent {
		AssetReference<TextureAsset> specularTexture;

		REFLECT("EnvironmentMap")
	};

	void SetupEnvironmentMapComponent(entt::registry&, entt::entity);
	void DestroyEnvironmentMapComponent(entt::registry&, entt::entity);
}
