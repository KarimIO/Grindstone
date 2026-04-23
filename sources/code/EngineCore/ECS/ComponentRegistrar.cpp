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

	auto srcEntityView = srcRegistry.view<entt::entity>();
	srcEntityView.each(
		[&dstRegistry](entt::entity srcEntity) {
			entt::entity dstEntity = dstRegistry.create(srcEntity);
			if (dstEntity == entt::null) {
				GPRINT_ERROR_V(LogSource::EngineCore, "Failure to create entity {}", static_cast<uint32_t>(dstEntity));
			}
		}
	);

	for (auto& fns : componentFunctionsList) {
		fns.second.copyRegistryComponentsFn(dst, src);
	}
}

void ComponentRegistrar::DestroyEntity(ECS::Entity entity) {
	Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
	entt::registry& registry = cxtSet.GetEntityRegistry();
	entt::entity entityHandle = entity.GetHandle();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;
		if (compFns.hasComponentFn(registry, entityHandle) && compFns.destroyComponentFn) {
			compFns.destroyComponentFn(cxtSet, entityHandle);
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
			if (compFns.setupComponentFn != nullptr && compFns.hasComponentFn(registry, entity)) {
				compFns.setupComponentFn(worldContextSet, entity);
			}
		}
	}
}

void ComponentRegistrar::CallDestroyOnRegistry(Grindstone::WorldContextSet& worldContextSet) {
	Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
	entt::registry& registry = cxtSet.GetEntityRegistry();
	auto entityView = registry.view<entt::entity>();

	for (auto& compFnPair : componentFunctionsList) {
		ComponentFunctions& compFns = compFnPair.second;

		if (compFns.destroyComponentFn != nullptr) {
			for (entt::entity entity : entityView) {
				if (compFns.hasComponentFn(registry, entity)) {
					compFns.destroyComponentFn(cxtSet, entity);
				}
			}
		}
	}
}

void ComponentRegistrar::RegisterComponent(Grindstone::HashedString name, ComponentFunctions componentFunctions) {
	auto comp = componentFunctionsList.find(name);
	if (comp != componentFunctionsList.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Registering a component that was already registered: {}", name.ToString());
	}

	componentFunctionsList.emplace(name, componentFunctions);
}

void ComponentRegistrar::UnregisterComponent(Grindstone::HashedString name) {
	auto comp = componentFunctionsList.find(name);
	if (comp == componentFunctionsList.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unregistering a component that isn't registered: {}", name.ToString());
	}

	componentFunctionsList.erase(comp);
}

void* ComponentRegistrar::CreateComponentWithSetup(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();

	auto& fns = selectedFactory->second;
	auto comp = fns.createComponentFn(registry, entity.GetHandle());

	if (fns.setupComponentFn) {
		Grindstone::WorldContextSet& cxtSet = GetActiveWorldContextSet();
		fns.setupComponentFn(cxtSet, entity.GetHandle());
	}

	return comp;
}

void* ComponentRegistrar::CreateComponentWithSetup(WorldContextSet& worldContextSet, Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();

	auto& fns = selectedFactory->second;
	auto comp = fns.createComponentFn(registry, entity.GetHandle());

	if (fns.setupComponentFn) {
		fns.setupComponentFn(worldContextSet, entity.GetHandle());
	}

	return comp;
}

void* ComponentRegistrar::CreateComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return nullptr;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.createComponentFn(registry, entity.GetHandle());
}

void ComponentRegistrar::RemoveComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.destroyComponentFn) {
		fns.destroyComponentFn(GetActiveWorldContextSet(), entity.GetHandle());
	}

	entt::registry& registry = GetEntityRegistry();
	fns.removeComponentFn(registry, entity.GetHandle());
}

bool ComponentRegistrar::HasComponent(Grindstone::HashedString name, ECS::Entity entity) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.hasComponentFn(registry, entity.GetHandle());
}

bool ComponentRegistrar::TryGetComponent(Grindstone::HashedString name, ECS::Entity entity, void*& outComponent) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	entt::registry& registry = GetEntityRegistry();
	return selectedFactory->second.tryGetComponentFn(registry, entity.GetHandle(), outComponent);
}

bool ComponentRegistrar::TryGetComponentReflectionData(Grindstone::HashedString name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return false;
	}

	outReflectionData = selectedFactory->second.getComponentReflectionDataFn();
	return true;
}

void ComponentRegistrar::SetupComponent(Grindstone::HashedString name, ECS::Entity entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.setupComponentFn) {
		fns.setupComponentFn(GetActiveWorldContextSet(), entity.GetHandle());
	}
}

void ComponentRegistrar::SetupComponent(WorldContextSet& worldContextSet, Grindstone::HashedString name, ECS::Entity entity, void* componentPtr) {
	auto selectedFactory = componentFunctionsList.find(name);
	if (selectedFactory == componentFunctionsList.end()) {
		return;
	}

	auto& fns = selectedFactory->second;
	if (fns.setupComponentFn) {
		fns.setupComponentFn(worldContextSet, entity.GetHandle());
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
