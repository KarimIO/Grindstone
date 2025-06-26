#include "CSharpSystem.hpp"
#include "CSharpManager.hpp"
using namespace Grindstone::Scripting;

void CSharp::UpdateSystem(entt::registry& registry) {
}

void CSharp::UpdateEditorSystem(entt::registry& registry) {
	CSharpManager::GetInstance().EditorUpdate(registry);
}
