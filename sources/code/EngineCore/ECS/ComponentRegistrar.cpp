#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>

#include "ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

static Grindstone::WorldContextSet& GetActiveWorldContextSet() {
	return *EngineCore::GetInstance().GetWorldContextManager()->GetActiveWorldContextSet();
}

static entt::registry& GetEntityRegistry() {
	return GetActiveWorldContextSet().GetEntityRegistry();
}

void ComponentRegistrar::CopyRegistry(WorldContextSet& dst, WorldContextSet& src) {
	const entt::registry& srcRegistry = src.GetEntityRegistry();
	entt::registry& dstRegistry = src.GetEntityRegistry();

	auto& srcEntityView = srcRegistry.view<entt::entity>();
	srcEntityView.each(
		[&dstRegistry](entt::entity srcEntity) {
			entt::entity dstEntity = dstRegistry.create(srcEntity);
			if (dstEntity == entt::null) {
				GPRINT_ERROR_V(LogSource::EngineCore, "Failure to create entity {}", static_cast<uint32_t>(dstEntity));
			}
		}
	);

	for (auto& fns : componentFunctionsList) {
		fns.second.CopyRegistryComponentsFn(dst, src);
	}
}

void ComponentRegistrar::DestroyEntity(ECS::Entity entity) {
	Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
	entt::registry& registry = cxtSet.GetEntityRegistry();
	entt::entity entityHandle = entity.GetHandle();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;
		if (compFns.HasComponentFn(registry, entityHandle) && compFns.DestroyComponentFn) {
			compFns.DestroyComponentFn(cxtSet, entityHandle);
		}
	}

	registry.destroy(entityHandle);
}

void ComponentRegistrar::CallCreateOnRegistry(Grindstone::WorldContextSet& worldContextSet) {
	entt::registry& registry = worldContextSet.GetEntityRegistry();
	auto& entityView = registry.view<entt::entity>();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;

		for (entt::entity entity : entityView) {
			if (compFns.SetupComponentFn != nullptr && compFns.HasComponentFn(registry, entity)) {
				compFns.SetupComponentFn(worldContextSet, entity);
			}
		}
	}
}

void ComponentRegistrar::CallDestroyOnRegistry(Grindstone::WorldContextSet& worldContextSet) {
	Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
	entt::registry& registry = cxtSet.GetEntityRegistry();
	auto& entityView = registry.view<entt::entity>();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;

		for (entt::entity entity : entityView) {
			if (compFns.DestroyComponentFn != nullptr && compFns.HasComponentFn(registry, entity)) {
				compFns.DestroyComponentFn(cxtSet, entity);
			}
		}
	}
}

void ComponentRegistrar::RegisterComponent(const char *name, ComponentFunctions componentFunctions) {
	auto comp = componentFunctionsList.find(name);
	if (comp != componentFunctionsList.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Registering a component that was already registered: {}", name);
	}

	componentFunctionsList.emplace(name, componentFunctions);
}

void ComponentRegistrar::UnregisterComponent(const char* name) {
	auto comp = componentFunctionsList.find(name);
	if (comp == componentFunctionsList.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unregistering a component that isn't registered: {}", name);
	}

	componentFunctionsList.erase(comp);
}

void* ComponentRegistrar::CreateComponentWithSetup(const char* name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();

	auto& fns = selectedFactory->second;
	auto comp = fns.CreateComponentFn(registry, entity.GetHandle());

	if (fns.SetupComponentFn) {
		Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
		fns.SetupComponentFn(cxtSet, entity.GetHandle());
	}

	return comp;
}

void* ComponentRegistrar::CreateComponentWithSetup(WorldContextSet& worldContextSet, const char* name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();

	auto& fns = selectedFactory->second;
	auto comp = fns.CreateComponentFn(registry, entity.GetHandle());

	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(worldContextSet, entity.GetHandle());
	}

	return comp;
}

void* ComponentRegistrar::CreateComponent(const char* name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.CreateComponentFn(registry, entity.GetHandle());
}

void ComponentRegistrar::RemoveComponent(const char *name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.DestroyComponentFn) {
		fns.DestroyComponentFn(GetActiveWorldContextSet(), entity.GetHandle());
	}

	entt::registry& registry = GetEntityRegistry();
	fns.RemoveComponentFn(registry, entity.GetHandle());
}

bool ComponentRegistrar::HasComponent(const char* name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.HasComponentFn(registry, entity.GetHandle());
}

bool ComponentRegistrar::TryGetComponent(const char* name, ECS::Entity entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.TryGetComponentFn(registry, entity.GetHandle(), outComponent);
}

bool ComponentRegistrar::TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.GetComponentReflectionDataFn();
	return true;
}

void ComponentRegistrar::SetupComponent(const char* componentType, ECS::Entity entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(componentType);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(GetActiveWorldContextSet(), entity.GetHandle());
	}
}

void ComponentRegistrar::SetupComponent(WorldContextSet& worldContextSet, const char* componentType, ECS::Entity entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(componentType);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.SetupComponentFn) {
		fns.SetupComponentFn(worldContextSet, entity.GetHandle());
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
