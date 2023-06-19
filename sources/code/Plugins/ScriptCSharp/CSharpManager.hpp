#pragma once

#include <entt/entt.hpp>
#include "mono/utils/mono-forward.h"
#include "mono/metadata/image.h"
#include "mono/metadata/metadata.h"

#include "EngineCore/ECS/ComponentFunctions.hpp"

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
				struct AssemblyData {
					MonoAssembly* assembly = nullptr;
					MonoImage* image = nullptr;
				};

				static CSharpManager& GetInstance();
				void Cleanup();
				virtual void Initialize(EngineCore* engineCore);
				virtual void CreateDomain();
				virtual void LoadAssembly(const char* path, AssemblyData& outAssemblyData);
				virtual void LoadAssemblyIntoMap(const char* path);
				virtual void SetupComponent(ECS::Entity& entity, ScriptComponent& component);
				virtual void CallStartInAllComponents(entt::registry& registry);
				virtual void CallUpdateInAllComponents(entt::registry& registry);
				virtual void CallEditorUpdateInAllComponents(entt::registry& registry);
				virtual void CallDeleteInAllComponents(entt::registry& registry);
				void RegisterComponents();
				void RegisterComponent(std::string& csharpClass, ECS::ComponentFunctions& fns);
				void CallCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType);
				void CallHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoTypes);
				void CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType);
				void QueueReload();
				void PerformReload();
				void EditorUpdate(entt::registry& registry);
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

				MonoDomain* rootDomain = nullptr;
				MonoDomain* scriptDomain = nullptr;
				EngineCore *engineCore = nullptr;
				std::map<std::string, AssemblyData> assemblies;
				AssemblyData grindstoneCoreDll;
				bool isReloadQueued = false;

				std::map<MonoType*, ECS::CreateComponentFn> createComponentFuncs;
				std::map<MonoType*, ECS::TryGetComponentFn> tryGetComponentFuncs;
				std::map<MonoType*, ECS::HasComponentFn> hasComponentFuncs;
				std::map<MonoType*, ECS::RemoveComponentFn> removeComponentFuncs;
			};
		}
	}
}
