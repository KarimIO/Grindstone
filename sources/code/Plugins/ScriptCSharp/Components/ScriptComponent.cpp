#include <Common/Math.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "../CSharpManager.hpp"
#include "ScriptComponent.hpp"

using namespace Grindstone::Scripting::CSharp;

REFLECT_STRUCT_BEGIN(ScriptComponent)
	REFLECT_STRUCT_MEMBER(assembly)
	REFLECT_STRUCT_MEMBER(scriptNamespace)
	REFLECT_STRUCT_MEMBER(scriptClass)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::Scripting::CSharp::SetupCSharpScriptComponent(
	Grindstone::WorldContextSet& cxtSet,
	entt::entity entity
) {
	CSharpManager& csManager = CSharpManager::GetInstance();
	ScriptComponent& component = cxtSet.GetEntityRegistry().get<ScriptComponent>(entity);
	csManager.SetupComponent(cxtSet, entity, component);
}

void Grindstone::Scripting::CSharp::DestroyCSharpScriptComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	CSharpManager& csManager = CSharpManager::GetInstance();
	ScriptComponent& component = cxtSet.GetEntityRegistry().get<ScriptComponent>(entity);
	csManager.DestroyComponent(cxtSet, entity, component);
}

