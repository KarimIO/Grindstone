#pragma once

#include <entt/entt.hpp>

#include <EngineCore/ECS/ComponentFunctions.hpp>
#include <Common/Buffer.hpp>

namespace Grindstone {
	class EngineCore;
	using AssemblyHash = int;

	namespace ECS {
		class Entity;
	}

	namespace Scripting::CSharp {
		struct ScriptComponent;
		struct ScriptClass;

		enum class InspectorFieldType {
			Unknown,
			Entity,
			Component,
			Float, Float2, Float3, Float4,
			Int, Bool, String,
			Quaternion,
			Color
		};

		struct FieldMetaData {
			using SetterCallback = void(*)(void* componentPtr, void* valuePtr);
			std::string name;
			std::string displayName;
			InspectorFieldType fieldType;
			uint32_t valueArenaOffset;
			uint32_t valueArenaSize;
			SetterCallback setterCallback = nullptr;
		};

		class CSharpManager {
		public:
			struct AssemblyData {
				AssemblyHash assemblyHash;
			};

			static CSharpManager& GetInstance();
			void Cleanup();
			virtual void Initialize();
			virtual bool LoadAssembly(const char* path, AssemblyData& outAssemblyData);
			virtual bool LoadAssemblyIntoMap(const std::string& assemblyIdentifier);
			virtual void SetupComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component);
			virtual void DestroyComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component);
			void RegisterComponents();
			void RegisterComponent(const Grindstone::String& csharpClass, ECS::ComponentFunctions& fns);
			void QueueReload();
			void PerformReload();
			void Update(entt::registry& registry);
			void EditorUpdate(entt::registry& registry);
			virtual std::pair<Grindstone::Buffer, std::vector<FieldMetaData>> GetFieldMetaData(ScriptComponent& component);
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
			std::map<std::string, ScriptClass*> classMetaData;
			AssemblyData grindstoneCoreDll;
			bool isReloadQueued = false;
		};
	}
}
