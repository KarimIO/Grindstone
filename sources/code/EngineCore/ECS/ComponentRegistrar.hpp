#pragma once

#include <unordered_map>
#include <string>
#include <entt/entt.hpp>

#include <EngineCore/ECS/EntityHandle.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <Common/HashedString.hpp>

#include "ComponentFunctions.hpp"
#include "ComponentFunctionsDefinitions.hpp"

using namespace Grindstone;

namespace Grindstone::ECS {
	class ComponentRegistrar {
	public:
		template<typename ComponentType>
		void RegisterComponent(SetupComponentFn setupComponentFn = nullptr, DestroyComponentFn destroyComponentFn = nullptr) {
			RegisterComponent(
				ComponentType::GetComponentHashString(),
				ComponentFunctions{
					setupComponentFn,
					destroyComponentFn,
					&ECS::CreateComponent<ComponentType>,
					&ECS::RemoveComponent<ComponentType>,
					&ECS::HasComponent<ComponentType>,
					&ECS::TryGetComponent<ComponentType>,
					&ECS::GetComponentReflectionData<ComponentType>,
					&ECS::CopyRegistryComponents<ComponentType>
				}
			);
		}

		template<typename ComponentType>
		void UnregisterComponent() {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextManager* worldContextManager = engineCore.GetWorldContextManager();
			if (worldContextManager != nullptr) {
				for (Grindstone::UniquePtr<Grindstone::WorldContextSet>& worldContext : *worldContextManager) {
					entt::registry& registry = worldContext.Get()->GetEntityRegistry();
					registry.clear<ComponentType>();
				}
			}

			auto comp = componentFunctionsList.find(ComponentType::GetComponentHashString());
			if (comp != componentFunctionsList.end()) {
				componentFunctionsList.erase(comp);
			}
		}

		virtual void CopyRegistry(WorldContextSet& to, WorldContextSet& from);
		virtual void CallCreateOnRegistry(WorldContextSet& worldContextSet);
		virtual void CallDestroyOnRegistry(WorldContextSet& worldContextSet);
		virtual void DestroyEntity(ECS::Entity entity);
		virtual void RegisterComponent(Grindstone::HashedString name, ComponentFunctions componentFunctions);
		virtual void UnregisterComponent(Grindstone::HashedString name);
		virtual void* CreateComponentWithSetup(Grindstone::HashedString name, ECS::Entity entity);
		virtual void* CreateComponentWithSetup(WorldContextSet& worldContextSet, Grindstone::HashedString name, ECS::Entity entity);
		virtual void* CreateComponent(Grindstone::HashedString name, ECS::Entity entity);
		virtual void RemoveComponent(Grindstone::HashedString name, ECS::Entity entity);
		virtual bool HasComponent(Grindstone::HashedString name, ECS::Entity entity);
		virtual bool TryGetComponent(Grindstone::HashedString name, ECS::Entity entity, void*& outComponent);
		virtual bool TryGetComponentReflectionData(Grindstone::HashedString name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData);
		virtual void SetupComponent(Grindstone::HashedString componentType, ECS::Entity entity, void* componentPtr);
		virtual void SetupComponent(WorldContextSet& worldContextSet, Grindstone::HashedString componentType, ECS::Entity entity, void* componentPtr);

		using ComponentMap = std::unordered_map<Grindstone::HashedString, ComponentFunctions>;
		virtual ComponentMap::iterator begin();
		virtual ComponentMap::const_iterator begin() const;
		virtual ComponentMap::iterator end();
		virtual ComponentMap::const_iterator end() const;
	private:
		ComponentMap componentFunctionsList;
	};
}
