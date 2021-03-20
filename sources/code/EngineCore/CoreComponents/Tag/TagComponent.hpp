#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct TagComponent {
		std::string tag;

		REFLECT("Transform")
	};
}
