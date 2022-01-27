#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "setupCoreComponents.hpp"

#include "Tag/TagComponent.hpp"
#include "Camera/CameraComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "Mesh/MeshComponent.hpp"
#include "Mesh/MeshRendererComponent.hpp"
#include "Lights/PointLightComponent.hpp"

using namespace Grindstone;

void Grindstone::SetupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->RegisterComponent<Grindstone::TagComponent>();
	registrar->RegisterComponent<Grindstone::TransformComponent>();
	registrar->RegisterComponent<Grindstone::CameraComponent>(SetupCameraComponent);
	registrar->RegisterComponent<Grindstone::MeshComponent>();
	registrar->RegisterComponent<Grindstone::MeshRendererComponent>();
	registrar->RegisterComponent<Grindstone::PointLightComponent>();
}
