#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct AudioSourceComponent {
		bool isLooping;
		bool isActive;
		float volume;
		float pitch;
		REFLECT("AudioSource")
	};
}
