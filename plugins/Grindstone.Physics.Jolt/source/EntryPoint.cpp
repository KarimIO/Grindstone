#include <Grindstone.Physics.Jolt/include/pch.hpp>
#include <chrono>
#include <string>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

template<typename ComponentType>
void SetupColliderComponent(Grindstone::WorldContextSet& cxt, entt::entity entity) {
	entt::registry& registry = cxt.GetEntityRegistry();
	ComponentType& colliderComponent = registry.get<ComponentType>(entity);

	colliderComponent.Initialize();

	RigidBodyComponent* rigidBodyComponent = registry.try_get<RigidBodyComponent>(entity);
	TransformComponent* transformComponent = registry.try_get<TransformComponent>(entity);
	if (rigidBodyComponent != nullptr && transformComponent != nullptr) {
		SetupRigidBodyComponentWithCollider(
			cxt,
			rigidBodyComponent,
			transformComponent,
			&colliderComponent
		);
	}
}

extern "C" {
	JOLT_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		JPH::RegisterTypes();

		pluginInterface->RegisterComponent<BoxColliderComponent>(
			SetupColliderComponent<BoxColliderComponent>
		);

		pluginInterface->RegisterComponent<SphereColliderComponent>(
			SetupColliderComponent<SphereColliderComponent>
		);

		pluginInterface->RegisterComponent<PlaneColliderComponent>(
			SetupColliderComponent<PlaneColliderComponent>
		);

		pluginInterface->RegisterComponent<CapsuleColliderComponent>(
			SetupColliderComponent<CapsuleColliderComponent>
		);

		pluginInterface->RegisterWorldContextFactory<Grindstone::Physics::WorldContext>(physicsWorldContextName);
		pluginInterface->RegisterComponent<RigidBodyComponent>(SetupRigidBodyComponent);
		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsJoltSystem);
	}

	JOLT_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterSystem("PhysicsSystem");
		pluginInterface->UnregisterComponent<RigidBodyComponent>();
		pluginInterface->UnregisterWorldContextFactory(physicsWorldContextName);

		pluginInterface->UnregisterComponent<CapsuleColliderComponent>();
		pluginInterface->UnregisterComponent<PlaneColliderComponent>();
		pluginInterface->UnregisterComponent<SphereColliderComponent>();
		pluginInterface->UnregisterComponent<BoxColliderComponent>();

		JPH::UnregisterTypes();
	}
}
