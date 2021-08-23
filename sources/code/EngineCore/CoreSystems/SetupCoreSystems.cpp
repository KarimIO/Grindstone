#include "../ECS/SystemRegistrar.hpp"
#include "SetupCoreSystems.hpp"
#include "RenderSystem.hpp"
using namespace Grindstone;

void Grindstone::SetupCoreSystems(ECS::SystemRegistrar* registrar) {
	registrar->RegisterSystem("Render", RenderSystem);
}
