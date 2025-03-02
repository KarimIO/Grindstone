#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EnvironmentMapComponent.hpp"
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/ECS/Entity.hpp>
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(EnvironmentMapComponent)
	REFLECT_STRUCT_MEMBER(specularTexture)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupEnvironmentMapComponent(entt::registry& registry, entt::entity entity) {
	EnvironmentMapComponent& envMap = registry.get<EnvironmentMapComponent>(entity);
	envMap.specularTexture = Grindstone::AssetReference<Grindstone::TextureAsset>::CreateAndIncrement(envMap.specularTexture.uuid);
}

void Grindstone::DestroyEnvironmentMapComponent(entt::registry& registry, entt::entity entity) {
	EnvironmentMapComponent& envMap = registry.get<EnvironmentMapComponent>(entity);
	// TODO: Unload environment map
}
