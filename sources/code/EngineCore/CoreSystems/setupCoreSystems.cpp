#include "../ECS/SystemRegistrar.hpp"
#include "setupCoreSystems.hpp"
#include "exampleSystem.hpp"
using namespace Grindstone;

void Grindstone::setupCoreSystems(ECS::SystemRegistrar* registrar) {
	registrar->registerSystem("Example", exampleSystem);
}
