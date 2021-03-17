#include "Transform/createTransformComponent.hpp"
#include "../ECS/ComponentRegistrar.hpp"
#include "setupCoreComponents.hpp"
using namespace Grindstone;

void Grindstone::setupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->registerComponent("Transform", createTransformComponent);
}
