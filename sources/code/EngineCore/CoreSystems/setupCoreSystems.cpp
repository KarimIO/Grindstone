#include "../ECS/SystemRegistrar.hpp"
#include "setupCoreSystems.hpp"
#include "renderSystem.hpp"
#include "exampleSystem.hpp"
using namespace Grindstone;

void Grindstone::setupCoreSystems(ECS::SystemRegistrar* registrar) {
	registrar->registerSystem("Render", renderSystem);
	registrar->registerSystem("Example", exampleSystem);
}
