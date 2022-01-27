#pragma once

#include <string>
#include <entt/entt.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Plugins/ScriptCSharp/ScriptClass.hpp"

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptComponent {
				std::string assembly;
				std::string scriptNamespace;
				std::string scriptClass;
				ScriptClass* monoClass = nullptr;
				void* scriptObject = nullptr;

				REFLECT("CSharpScript")
			};

			void SetupCSharpScriptComponent(entt::registry& registry, entt::entity entity, void* componentPtr);
		}
	}
}
