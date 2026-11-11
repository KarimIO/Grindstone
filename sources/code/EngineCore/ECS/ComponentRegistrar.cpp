#include <Common/Assert.hpp>
#include <Common/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>

#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

void ComponentRegistrar::CopyRegistry(WorldContextSet& dst, WorldContextSet& src) {
	const entt::registry& srcRegistry = src.GetEntityRegistry();
	entt::registry& dstRegistry = dst.GetEntityRegistry();

	auto srcEntityView = srcRegistry.view<entt::entity>();
	srcEntityView.each(
		[&dstRegistry](entt::entity entityID) {
			entt::entity dstEntity = dstRegistry.create(entityID);
			if (dstEntity == entt::null) {
				GPRINT_ERROR(LogSource::EngineCore, "Failure to create entity {}", static_cast<uint32_t>(dstEntity));
			}
			else if (dstEntity != entityID) {
				GPRINT_ERROR(LogSource::EngineCore, "New entity {} != old entity {}", static_cast<uint32_t>(dstEntity), static_cast<uint32_t>(entityID));
			}
		}
	);

	for (auto& fns : componentFunctionsList) {
		fns.second.CopyRegistryComponentsFn(dst, src);
	}
}

void ComponentRegistrar::DestroyEntity(ECS::Entity entity) {
	Grindstone::WorldContextSet* worldContextSet = entity.GetWorldContextSet();
	entt::registry& registry = worldContextSet-> GetEntityRegistry();
	entt::entity entityHandle = entity.GetHandle();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;
		if (compFns.HasComponentFn(*worldContextSet, entityHandle) && compFns.DestroyComponentFn) {
			compFns.DestroyComponentFn(*worldContextSet, entityHandle);
		}
	}

	registry.destroy(entityHandle);
}

void ComponentRegistrar::CallCreateOnRegistry(Grindstone::WorldContextSet& worldContextSet) {
	entt::registry& registry = worldContextSet.GetEntityRegistry();
	auto entityView = registry.view<entt::entity>();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;

		for (entt::entity entity : entityView) {
			if (compFns.SetupComponentFn != nullptr && compFns.HasComponentFn(worldContextSet, entity)) {
				compFns.SetupComponentFn(worldContextSet, entity);
			}
		}
	}
}

void ComponentRegistrar::CallDestroyOnRegistry(Grindstone::WorldContextSet& worldContextSet) {
	entt::registry& registry = worldContextSet.GetEntityRegistry();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;

		if (compFns.DestroyComponentFn != nullptr) {
			for (entt::entity entity : registry.storage<entt::entity>()) {
				if (compFns.HasComponentFn(worldContextSet, entity)) {
					compFns.DestroyComponentFn(worldContextSet, entity);
				}
			}
		}
	}
}

void ComponentRegistrar::RegisterComponent(Grindstone::HashedString name, ComponentFunctions componentFunctions) {
	auto comp = componentFunctionsList.find(name);
	if (comp != componentFunctionsList.end()) {
		GPRINT_ERROR(LogSource::EngineCore, "Registering a component that was already registered: {}", name.ToString());
	}

	componentFunctionsList.emplace(name, componentFunctions);
}

void ComponentRegistrar::UnregisterComponent(Grindstone::HashedString name) {
	auto comp = componentFunctionsList.find(name);
	if (comp == componentFunctionsList.end()) {
		GPRINT_ERROR(LogSource::EngineCore, "Unregistering a component that isn't registered: {}", name.ToString());
	}

	componentFunctionsList.erase(comp);
}

void* ComponentRegistrar::CreateComponentWithSetup(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	Grindstone::WorldContextSet& worldContextSet = *entity.GetWorldContextSet();
	entt::registry& registry = worldContextSet.GetEntityRegistry();

	auto& fns = selectedFactory->second;
	auto comp = fns.CreateComponentFn(worldContextSet, entity.GetHandle());

	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(worldContextSet, entity.GetHandle());
	}

	return comp;
}

// TODO: Add WorldContextSet?
void* ComponentRegistrar::CreateComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	Grindstone::WorldContextSet* worldContextSet = entity.GetWorldContextSet();
	entt::registry& registry = worldContextSet->GetEntityRegistry();
	return selectedFactory->second.CreateComponentFn(*worldContextSet, entity.GetHandle());
}

// TODO: Add WorldContextSet?
void ComponentRegistrar::RemoveComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.DestroyComponentFn) {
		fns.DestroyComponentFn(*entity.GetWorldContextSet(), entity.GetHandle());
	}

	fns.RemoveComponentFn(*entity.GetWorldContextSet(), entity.GetHandle());
}

bool ComponentRegistrar::HasComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.HasComponentFn(*entity.GetWorldContextSet(), entity.GetHandle());
}

bool ComponentRegistrar::TryGetComponent(Grindstone::HashedString name, ECS::Entity entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	return selectedFactory->second.TryGetComponentFn(*entity.GetWorldContextSet(), entity.GetHandle(), outComponent);
}

bool ComponentRegistrar::TryGetComponentReflectionData(Grindstone::HashedString name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.GetComponentReflectionDataFn();
	return true;
}

void ComponentRegistrar::SetupComponent(Grindstone::HashedString name, ECS::Entity entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.SetupComponentFn) {
		Grindstone::WorldContextSet* worldContexSet = entity.GetWorldContextSet();
		fns.SetupComponentFn(*worldContexSet, entity.GetHandle());
	}
}

ComponentRegistrar::ComponentMap::iterator ComponentRegistrar::begin() {
	return componentFunctionsList.begin();
}

ComponentRegistrar::ComponentMap::const_iterator ComponentRegistrar::begin() const {
	return componentFunctionsList.begin();
}

ComponentRegistrar::ComponentMap::iterator ComponentRegistrar::end() {
	return componentFunctionsList.end();
}

ComponentRegistrar::ComponentMap::const_iterator ComponentRegistrar::end() const {
	return componentFunctionsList.end();
}

Grindstone::ECS::ComponentFunctions& ComponentRegistrar::GetComponentAccessFunctions(Grindstone::HashedString componentTypeName) {
	auto it = componentFunctionsList.find(componentTypeName);
	GS_ASSERT(it != componentFunctionsList.end());
	return componentFunctionsList[componentTypeName];
}

extern "C" {
	struct ComponentAccessBindings {
		CreateComponentFn createComponent;
		TryGetComponentFn tryGetComponent;
		HasComponentFn hasComponent;
		DestroyComponentFn destroyComponent;
	};

	ComponentAccessBindings GetComponentAccessBindings(const char* componentName) {
		Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
		Grindstone::ECS::ComponentRegistrar* compReg = engineCore.GetComponentRegistrar();
		GS_ASSERT(compReg);
		ComponentFunctions& compFns = compReg->GetComponentAccessFunctions(Grindstone::HashedString(componentName));

		return {
			compFns.CreateComponentFn,
			compFns.TryGetComponentFn,
			compFns.HasComponentFn,
			compFns.DestroyComponentFn
		};
	}
}
