#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Audio/AudioSource.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct AudioSourceComponent {
		std::string audioClipPath;
		bool isLooping = false;
		float volume = 1.f;
		float pitch = 1.f;
		Audio::Source* source = nullptr;

		REFLECT("AudioSource")
	};
}
