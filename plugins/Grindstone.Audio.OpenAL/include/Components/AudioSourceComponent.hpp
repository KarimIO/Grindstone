#pragma once

#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <Common/Math.hpp>
#include <Grindstone.Audio.OpenAL/include/Source.hpp>
#include <Grindstone.Audio.OpenAL/include/AudioClip.hpp>

namespace Grindstone {
	class WorldContextSet;

	struct AudioSourceComponent {
		
		static void Construct(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);

		void Play();
		void Pause();
		void Stop();

		AssetReference<Audio::AudioClipAsset> audioClip;
		bool playOnStart = false;
		bool isLooping = false;
		float volume = 1.f;
		float pitch = 1.f;
		Audio::Source* source = nullptr;

		REFLECT("AudioSource")
	};
}
