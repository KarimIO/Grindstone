#pragma once

#include <entt/entt.hpp>
#include "mono/utils/mono-forward.h"
#include "mono/metadata/image.h"
#include "mono/metadata/object-forward.h"

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			struct ScriptComponent;
			struct ScriptClass;

			class CSharpManager {
			public:
				CSharpManager();
				static CSharpManager& GetInstance();
				virtual void Initialize();
				virtual void LoadAssembly(const char* path);
				virtual void SetupComponent(ScriptComponent& component);
				virtual void CallStartInAllComponents(entt::registry& registry);
				virtual void CallUpdateInAllComponents(entt::registry& registry);
				virtual void CallDeleteInAllComponents(entt::registry& registry);
			private:
				void CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset);
				void CallInitializeInComponent(ScriptComponent& scriptComponent);
				void CallStartInComponent(ScriptComponent& scriptComponent);
				void CallUpdateInComponent(ScriptComponent& scriptComponent);
				void CallDeleteInComponent(ScriptComponent& scriptComponent);
				ScriptClass* SetupClass(const char* assemblyName, const char* namespaceName, const char* className);

				struct AssemblyData {
					MonoAssembly* assembly = nullptr;
					MonoImage* image = nullptr;
				};

				MonoDomain* scriptDomain = nullptr;
				std::map<std::string, AssemblyData> assemblies;
			};
		}
	}
}
