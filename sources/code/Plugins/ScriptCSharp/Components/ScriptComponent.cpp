#include "Common/Math.hpp"
#include "ScriptComponent.hpp"
#include "../CSharpManager.hpp"
using namespace Grindstone::Scripting::CSharp;

REFLECT_STRUCT_BEGIN(ScriptComponent)
	REFLECT_STRUCT_MEMBER(assembly)
	REFLECT_STRUCT_MEMBER(scriptNamespace)
	REFLECT_STRUCT_MEMBER(scriptClass)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::Scripting::CSharp::SetupCSharpScriptComponent(
	entt::registry& registry,
	entt::entity entity
) {
	CSharpManager& csManager = CSharpManager::GetInstance();
	ScriptComponent& component = registry.get<ScriptComponent>(entity);
	csManager.SetupComponent(registry, entity, component);
}

void Grindstone::Scripting::CSharp::DestroyCSharpScriptComponent(entt::registry& registry, entt::entity entity) {
	CSharpManager& csManager = CSharpManager::GetInstance();
	ScriptComponent& component = registry.get<ScriptComponent>(entity);
	csManager.DestroyComponent(registry, entity, component);
}

