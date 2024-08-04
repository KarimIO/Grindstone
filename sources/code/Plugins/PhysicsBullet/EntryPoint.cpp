#include "pch.hpp"
#include <chrono>
#include <string>

#include <btBulletDynamicsCommon.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include "Components/ColliderComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "PhysicsSystem.hpp"
#include "Core.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

Physics::Core* physicsCore = nullptr;

template<typename ComponentType>
void SetupColliderComponent(entt::registry& registry, entt::entity entity) {
	ComponentType& colliderComponent = registry.get<ComponentType>(entity);

	if (colliderComponent.collisionShape == nullptr) {
		colliderComponent.Initialize();
		colliderComponent.collisionShape->setUserPointer(&colliderComponent);
	}

	RigidBodyComponent* rigidBodyComponent = registry.try_get<RigidBodyComponent>(entity);
	TransformComponent* transformComponent = registry.try_get<TransformComponent>(entity);
	if (rigidBodyComponent != nullptr && transformComponent != nullptr) {
		SetupRigidBodyComponentWithCollider(
			rigidBodyComponent,
			transformComponent,
			&colliderComponent
		);
	}
}

template<typename ComponentType>
void DestroyColliderComponent(entt::registry& registry, entt::entity entity) {
	ComponentType& colliderComponent = registry.get<ComponentType>(entity);

	AllocatorCore::Free(colliderComponent.collisionShape);
}


extern "C" {
	BULLET_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		physicsCore = AllocatorCore::Allocate<Physics::Core>();

		pluginInterface->RegisterComponent<BoxColliderComponent>(
			SetupColliderComponent<BoxColliderComponent>,
			DestroyColliderComponent<BoxColliderComponent>
		);

		pluginInterface->RegisterComponent<SphereColliderComponent>(
			SetupColliderComponent<SphereColliderComponent>,
			DestroyColliderComponent<SphereColliderComponent>
		);

		pluginInterface->RegisterComponent<PlaneColliderComponent>(
			SetupColliderComponent< PlaneColliderComponent>,
			DestroyColliderComponent<PlaneColliderComponent>
		);

		pluginInterface->RegisterComponent<CapsuleColliderComponent>(
			SetupColliderComponent<CapsuleColliderComponent>,
			DestroyColliderComponent< CapsuleColliderComponent>
		);

		pluginInterface->RegisterComponent<RigidBodyComponent>(SetupRigidBodyComponent, DestroyRigidBodyComponent);

		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsBulletSystem);
	}

	BULLET_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterSystem("PhysicsSystem");

		pluginInterface->UnregisterComponent<RigidBodyComponent>();

		pluginInterface->UnregisterComponent<CapsuleColliderComponent>();
		pluginInterface->UnregisterComponent<PlaneColliderComponent>();
		pluginInterface->UnregisterComponent<SphereColliderComponent>();
		pluginInterface->UnregisterComponent<BoxColliderComponent>();


		AllocatorCore::Free(physicsCore);
	}
}
