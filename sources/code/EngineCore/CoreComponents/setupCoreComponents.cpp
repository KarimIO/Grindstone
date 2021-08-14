#include "Tag/TagComponent.hpp"
#include "Camera/CameraComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "setupCoreComponents.hpp"
#include "../ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/ComponentFunctions.hpp"
using namespace Grindstone;

void Grindstone::SetupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->RegisterComponent<Grindstone::TagComponent>("Tag");
	registrar->RegisterComponent<Grindstone::TransformComponent>("Transform");
	registrar->RegisterComponent<Grindstone::CameraComponent>("Camera");
}
