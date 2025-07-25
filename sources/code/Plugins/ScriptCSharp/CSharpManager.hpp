#pragma once

#include <entt/entt.hpp>

#include "EngineCore/ECS/ComponentFunctions.hpp"

namespace Grindstone {
	class EngineCore;
	using AssemblyHash = int;

	namespace ECS {
		class Entity;
	}

	namespace Scripting::CSharp {
		struct ScriptComponent;
		struct ScriptClass;

		class CSharpManager {
		public:
			struct AssemblyData {
				AssemblyHash assemblyHash;
			};

			static CSharpManager& GetInstance();
			void Cleanup();
			virtual void Initialize();
			virtual void LoadAssembly(const char* path, AssemblyData& outAssemblyData);
			virtual void LoadAssemblyIntoMap(const char* path);
			virtual void SetupComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component);
			virtual void DestroyComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component);
			void RegisterComponents();
			void RegisterComponent(const Grindstone::String& csharpClass, ECS::ComponentFunctions& fns);
			void CallCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void CallHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void QueueReload();
			void PerformReload();
			void Update(entt::registry& registry);
			void EditorUpdate(entt::registry& registry);
		private:
			void LoadAssemblyClasses();
			void CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset);
			void CallConstructorInComponent(ScriptComponent& scriptComponent);
			void CallAttachComponentInComponent(ScriptComponent& scriptComponent);
			void CallStartInComponent(ScriptComponent& scriptComponent);
			void CallUpdateInComponent(ScriptComponent& scriptComponent);
			void CallEditorUpdateInComponent(ScriptComponent& scriptComponent);
			void CallDeleteInComponent(ScriptComponent& scriptComponent);
			ScriptClass* SetupClass(const char* assemblyName, const char* namespaceName, const char* className);
			
			std::map<std::string, AssemblyData> assemblies;
			std::map<std::string, ScriptClass*> smartComponents;
			AssemblyData grindstoneCoreDll;
			bool isReloadQueued = false;

			// std::map<MonoType*, ECS::CreateComponentFn> createComponentFuncs;
			// std::map<MonoType*, ECS::TryGetComponentFn> tryGetComponentFuncs;
			// std::map<MonoType*, ECS::HasComponentFn> hasComponentFuncs;
			// std::map<MonoType*, ECS::RemoveComponentFn> removeComponentFuncs;
		};
	}
}
