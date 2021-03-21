#include "Tag/TagComponent.hpp"
#include "Camera/CameraComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "setupCoreComponents.hpp"
#include "../ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/ComponentFunctions.hpp"
using namespace Grindstone;

void Grindstone::setupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->registerComponent<Grindstone::TagComponent>("Tag");
	registrar->registerComponent<Grindstone::TransformComponent>("Transform");
	registrar->registerComponent<Grindstone::CameraComponent>("Camera");
}
