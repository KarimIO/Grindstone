#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "ParentComponent.hpp"

using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API uint32_t EntityGetParent(Grindstone::WorldContextSet* cxtSet, uint32_t entity) {
		entt::registry& reg = cxtSet->GetEntityRegistry();
		entt::entity parentEntity = reg.get<ParentComponent>((entt::entity)entity).parentEntity;
		return static_cast<uint32_t>(parentEntity);
	}

	ENGINE_CORE_API void EntitySetParent(Grindstone::WorldContextSet* cxtSet, uint32_t entity, uint32_t newParentEntity) {
		entt::registry& reg = cxtSet->GetEntityRegistry();
		reg.get<ParentComponent>((entt::entity)entity).parentEntity = static_cast<entt::entity>(newParentEntity);
	}
}

REFLECT_STRUCT_BEGIN(ParentComponent)
	REFLECT_STRUCT_MEMBER(parentEntity)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
