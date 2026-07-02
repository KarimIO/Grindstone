#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Renderables.3D/include/Components/AnimatorComponent.hpp>
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(AnimatorComponent)
	REFLECT_STRUCT_MEMBER(animation)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void AnimatorComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	AnimatorComponent& animComponent = cxtSet.GetEntityRegistry().get<AnimatorComponent>(entity);
	animComponent.animation.Release();
}
