#pragma once

#include <entt/entt.hpp>

#include "EngineCore/ECS/ComponentFunctions.hpp"

namespace Grindstone {
	class EngineCore;

	namespace ECS {
		class Entity;
	}

	namespace Scripting::CSharp {
		struct ScriptComponent;
		struct ScriptClass;

		class CSharpManager {
		public:
			struct AssemblyData {
				void* library;
			};

			static CSharpManager& GetInstance();
			void Cleanup();
			virtual void Initialize(EngineCore* engineCore);
			virtual void LoadAssembly(const char* path, AssemblyData& outAssemblyData);
			virtual void LoadAssemblyIntoMap(const char* path);
			virtual void SetupComponent(entt::registry& registry, entt::entity entity, ScriptComponent& component);
			virtual void DestroyComponent(entt::registry& registry, entt::entity entity, ScriptComponent& component);
			void RegisterComponents();
			void RegisterComponent(std::string& csharpClass, ECS::ComponentFunctions& fns);
			void CallCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void CallHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle);
			void QueueReload();
			void PerformReload();
			void EditorUpdate(entt::registry& registry);
		private:
			void LoadAssemblyClasses();
			void SetupEntityDataInComponent(entt::entity entity, ScriptComponent& component);
			void CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset);
			void CallConstructorInComponent(ScriptComponent& scriptComponent);
			void CallAttachComponentInComponent(ScriptComponent& scriptComponent);
			void CallStartInComponent(ScriptComponent& scriptComponent);
			void CallUpdateInComponent(ScriptComponent& scriptComponent);
			void CallEditorUpdateInComponent(ScriptComponent& scriptComponent);
			void CallDeleteInComponent(ScriptComponent& scriptComponent);
			ScriptClass* SetupClass(const char* assemblyName, const char* namespaceName, const char* className);
			
			EngineCore *engineCore = nullptr;
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
