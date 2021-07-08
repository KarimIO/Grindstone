#include "../ECS/SystemRegistrar.hpp"
#include "setupCoreSystems.hpp"
#include "renderSystem.hpp"
#include "exampleSystem.hpp"
using namespace Grindstone;

void Grindstone::setupCoreSystems(ECS::SystemRegistrar* registrar) {
	registrar->RegisterSystem("Render", renderSystem);
	registrar->RegisterSystem("Example", exampleSystem);
}
