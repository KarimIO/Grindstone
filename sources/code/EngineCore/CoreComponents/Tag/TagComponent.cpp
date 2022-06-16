#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TagComponent.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API void* EntityGetTagComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = scene->GetEntityRegistry();
		return &reg.get<TagComponent>((entt::entity)entity);
	}

	ENGINE_CORE_API const char* TagComponentGetTag(TagComponent& component) {
		return component.tag.c_str();
	}

	ENGINE_CORE_API void TagComponentSetTag(TagComponent& component, const char* tag) {
		component.tag = tag;
	}
}

REFLECT_STRUCT_BEGIN(TagComponent)
	REFLECT_STRUCT_MEMBER(tag)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
