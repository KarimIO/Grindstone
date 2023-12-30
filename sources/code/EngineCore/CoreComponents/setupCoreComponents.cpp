#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "setupCoreComponents.hpp"

#include "Tag/TagComponent.hpp"
#include "Parent/ParentComponent.hpp"
#include "EnvironmentMap/EnvironmentMapComponent.hpp"
#include "Camera/CameraComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "Lights/PointLightComponent.hpp"
#include "Lights/SpotLightComponent.hpp"
#include "Lights/DirectionalLightComponent.hpp"

using namespace Grindstone;

void Grindstone::SetupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->RegisterComponent<Grindstone::TagComponent>();
	registrar->RegisterComponent<Grindstone::TransformComponent>();
	registrar->RegisterComponent<Grindstone::ParentComponent>();
	registrar->RegisterComponent<Grindstone::CameraComponent>(SetupCameraComponent);
	registrar->RegisterComponent<Grindstone::SpotLightComponent>(SetupSpotLightComponent);
	registrar->RegisterComponent<Grindstone::PointLightComponent>(SetupPointLightComponent);
	registrar->RegisterComponent<Grindstone::DirectionalLightComponent>(SetupDirectionalLightComponent);
	registrar->RegisterComponent<Grindstone::EnvironmentMapComponent>(SetupEnvironmentMapComponent);
}
