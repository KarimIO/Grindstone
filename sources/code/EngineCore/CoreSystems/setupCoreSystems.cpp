#include "../ECS/SystemRegistrar.hpp"
#include "setupCoreSystems.hpp"
#include "renderSystem.hpp"
#include "exampleSystem.hpp"
using namespace Grindstone;

void Grindstone::SetupCoreSystems(ECS::SystemRegistrar* registrar) {
	registrar->RegisterSystem("Render", RenderSystem);
	registrar->RegisterSystem("Example", ExampleSystem);
}
