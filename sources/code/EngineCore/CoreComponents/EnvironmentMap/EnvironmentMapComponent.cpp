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

void Grindstone::SetupEnvironmentMapComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = Grindstone::EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	auto envMap = static_cast<EnvironmentMapComponent*>(componentPtr);
	envMap->specularTexture.Load(envMap->specularTexture.uuid);
}
