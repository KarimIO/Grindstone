#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"
#include <entt/entt.hpp>
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

	void SetupAudioSourceComponent(entt::registry& registry, entt::entity entity, void* componentPtr);
}
