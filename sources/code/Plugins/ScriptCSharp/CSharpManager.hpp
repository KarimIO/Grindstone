#pragma once

#include <entt/entt.hpp>
#include "mono/utils/mono-forward.h"
#include "mono/metadata/image.h"
#include "mono/metadata/object-forward.h"

namespace Grindstone {
	class EngineCore;

	namespace ECS {
		class Entity;
	}

	namespace Scripting {
		namespace CSharp {
			struct ScriptComponent;
			struct ScriptClass;

			class CSharpManager {
			public:
				static CSharpManager& GetInstance();
				virtual void Initialize(EngineCore* engineCore);
				virtual void LoadAssembly(const char* path);
				virtual void SetupComponent(ECS::Entity& entity, ScriptComponent& component);
				virtual void CallStartInAllComponents(entt::registry& registry);
				virtual void CallUpdateInAllComponents(entt::registry& registry);
				virtual void CallEditorUpdateInAllComponents(entt::registry& registry);
				virtual void CallDeleteInAllComponents(entt::registry& registry);
			private:
				void SetupEntityDataInComponent(ECS::Entity& entity, ScriptComponent& component);
				void CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset);
				void CallConstructorInComponent(ScriptComponent& scriptComponent);
				void CallAttachComponentInComponent(ScriptComponent& scriptComponent);
				void CallStartInComponent(ScriptComponent& scriptComponent);
				void CallUpdateInComponent(ScriptComponent& scriptComponent);
				void CallEditorUpdateInComponent(ScriptComponent& scriptComponent);
				void CallDeleteInComponent(ScriptComponent& scriptComponent);
				ScriptClass* SetupClass(const char* assemblyName, const char* namespaceName, const char* className);

				struct AssemblyData {
					MonoAssembly* assembly = nullptr;
					MonoImage* image = nullptr;
				};

				MonoDomain* scriptDomain = nullptr;
				EngineCore *engineCore = nullptr;
				std::map<std::string, AssemblyData> assemblies;
			};
		}
	}
}
