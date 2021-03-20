#include "Tag/createTagComponent.hpp"
#include "Transform/createTransformComponent.hpp"
#include "../ECS/ComponentRegistrar.hpp"
#include "setupCoreComponents.hpp"
using namespace Grindstone;

void Grindstone::setupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->registerComponent("Tag", createTagComponent);
	registrar->registerComponent("Transform", createTransformComponent);
}
