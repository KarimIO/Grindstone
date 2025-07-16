#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"
#include "../Source.hpp"
#include "../AudioClip.hpp"

namespace Grindstone {
	class WorldContextSet;

	struct AudioSourceComponent {
		AssetReference<Audio::AudioClipAsset> audioClip;
		bool isLooping = false;
		float volume = 1.f;
		float pitch = 1.f;
		Audio::Source* source = nullptr;

		REFLECT("AudioSource")
	};

	void SetupAudioSourceComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroyAudioSourceComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
