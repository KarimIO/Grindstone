#include "Tag/TagComponent.hpp"
#include "Camera/CameraComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "Mesh/MeshComponent.hpp"
#include "Mesh/MeshRendererComponent.hpp"
#include "Lights/PointLightComponent.hpp"
#include "Audio/AudioListenerComponent.hpp"
#include "Audio/AudioSourceComponent.hpp"

#include "setupCoreComponents.hpp"

#include "../ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/ComponentFunctions.hpp"
using namespace Grindstone;

void Grindstone::SetupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->RegisterComponent<Grindstone::TagComponent>();
	registrar->RegisterComponent<Grindstone::TransformComponent>();
	registrar->RegisterComponent<Grindstone::CameraComponent>();
	registrar->RegisterComponent<Grindstone::MeshComponent>();
	registrar->RegisterComponent<Grindstone::MeshRendererComponent>();
	registrar->RegisterComponent<Grindstone::PointLightComponent>();
	registrar->RegisterComponent<Grindstone::AudioListenerComponent>();
	registrar->RegisterComponent<Grindstone::AudioSourceComponent>();
}
