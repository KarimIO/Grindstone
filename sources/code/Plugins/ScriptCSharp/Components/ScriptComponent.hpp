#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include <string>
#include "mono/metadata/object-forward.h"

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptComponent {
				std::string componentName;
				MonoClass* monoClass = nullptr;
				void* scriptObject = nullptr;;

				REFLECT("ScriptComponent")
			};
		}
	}
}
