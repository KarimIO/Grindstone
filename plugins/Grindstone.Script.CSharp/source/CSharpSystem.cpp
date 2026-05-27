#include <Grindstone.Script.CSharp/include/CSharpSystem.hpp>
#include <Grindstone.Script.CSharp/include/CSharpManager.hpp>
using namespace Grindstone::Scripting;

void CSharp::UpdateSystem(Grindstone::WorldContextSet& worldContextSet) {
	CSharpManager::GetInstance().Update(worldContextSet.GetEntityRegistry());
}

void CSharp::UpdateEditorSystem(Grindstone::WorldContextSet& worldContextSet) {
	CSharpManager::GetInstance().EditorUpdate(worldContextSet.GetEntityRegistry());
}
