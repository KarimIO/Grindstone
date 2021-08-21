#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct AudioListenerComponent {
		char garbage; // Remove this but this won't compile without it

		REFLECT("AudioListener")
	};
}
