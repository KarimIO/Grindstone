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
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EngineCore/Rendering/BaseRenderer.hpp"

#include "EngineCore/Audio/AudioClip.hpp"
#include "EngineCore/Audio/AudioCore.hpp"
#include "EngineCore/Audio/AudioSource.hpp"
using namespace Grindstone;

void SetupCameraComponent(entt::registry& registry, entt::entity entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	CameraComponent& cameraComponent = *(CameraComponent*)componentPtr;

	cameraComponent.renderer = engineCore.CreateRenderer();
	eventDispatcher->AddEventListener(
		Events::EventType::WindowResize,
		std::bind(&BaseRenderer::OnWindowResize, cameraComponent.renderer, std::placeholders::_1)
	);

	eventDispatcher->AddEventListener(
		Events::EventType::WindowResize,
		std::bind(&CameraComponent::OnWindowResize, &cameraComponent, std::placeholders::_1)
	);
}

void SetupAudioSourceComponent(entt::registry& registry, entt::entity entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto core = engineCore.audioCore;

	AudioSourceComponent& audioSource = *(AudioSourceComponent*)componentPtr;

	std::string path = std::string("../compiledAssets/") + audioSource.audioClip;
	Audio::Clip* clip = core->CreateClip(path.c_str());
	Audio::Source::CreateInfo audioSourceCreateInfo{};
	audioSourceCreateInfo.audioClip = clip;
	audioSourceCreateInfo.isLooping = audioSource.isLooping;
	audioSourceCreateInfo.volume = audioSource.volume;
	audioSourceCreateInfo.pitch = audioSource.pitch;
	Audio::Source* source = core->CreateSource(audioSourceCreateInfo);

	source->Play();
}

void Grindstone::SetupCoreComponents(ECS::ComponentRegistrar* registrar) {
	registrar->RegisterComponent<Grindstone::TagComponent>();
	registrar->RegisterComponent<Grindstone::TransformComponent>();
	registrar->RegisterComponent<Grindstone::CameraComponent>(SetupCameraComponent);
	registrar->RegisterComponent<Grindstone::MeshComponent>();
	registrar->RegisterComponent<Grindstone::MeshRendererComponent>();
	registrar->RegisterComponent<Grindstone::PointLightComponent>();
	registrar->RegisterComponent<Grindstone::AudioListenerComponent>();
	registrar->RegisterComponent<Grindstone::AudioSourceComponent>(SetupAudioSourceComponent);
}
