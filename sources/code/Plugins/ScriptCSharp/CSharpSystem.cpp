#include "CSharpSystem.hpp"
#include "CSharpManager.hpp"
using namespace Grindstone::Scripting;
void CSharp::UpdateSystem(entt::registry& registry) {
	CSharpManager::GetInstance().CallUpdateInAllComponents(registry);
}
