#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "AnimatorComponent.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API void* EntityGetAnimatorComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = scene->GetEntityRegistry();
		return &reg.Get<AnimatorComponent>((entt::entity)entity);
	}
}

REFLECT_STRUCT_BEGIN(AnimatorComponent)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
