#pragma once

#include <entt/entt.hpp>
#include "mono/utils/mono-forward.h"

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptComponent;

			class CSharpManager {
			public:
				virtual void Initialize();
				virtual void LoadAssembly(const char* path);
				virtual void GetClass(const char* className);
				virtual void CallInitializeInComponent(ScriptComponent& scriptComponent);
				virtual void CallStartInAllComponents(entt::registry& registry);
				virtual void CallUpdateInAllComponents(entt::registry& registry);
				virtual void CallCleanupInAllComponents(entt::registry& registry);
			private:
				MonoDomain* scriptDomain;
			};
		}
	}
}
