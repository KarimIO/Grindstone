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
	entt::entity entity,
	void* componentPtr
) {
	CSharpManager& csManager = CSharpManager::GetInstance();
	auto& component = *(ScriptComponent*)componentPtr;
	csManager.SetupComponent(component);
}
