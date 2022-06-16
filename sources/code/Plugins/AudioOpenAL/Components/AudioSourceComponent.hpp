#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"
#include "../Source.hpp"

namespace Grindstone {
	struct AudioSourceComponent {
		std::string audioClip;
		bool isLooping = false;
		float volume = 1.f;
		float pitch = 1.f;
		Audio::Source* source = nullptr;

		REFLECT("AudioSource")
	};

	void SetupAudioSourceComponent(ECS::Entity& entity, void* componentPtr);
}
