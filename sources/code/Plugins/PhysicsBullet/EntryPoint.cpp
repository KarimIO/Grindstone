#include "pch.hpp"
#include <chrono>
#include <string>

#include <btBulletDynamicsCommon.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>

#include "Components/ColliderComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "PhysicsSystem.hpp"
#include "Core.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

Physics::Core* physicsCore = nullptr;

extern "C" {
	BULLET_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		physicsCore = AllocatorCore::Allocate<Physics::Core>();

		pluginInterface->RegisterComponent<BoxColliderComponent>(SetupColliderComponent, DestroyColliderComponent);
		pluginInterface->RegisterComponent<SphereColliderComponent>(SetupColliderComponent, DestroyColliderComponent);
		pluginInterface->RegisterComponent<PlaneColliderComponent>(SetupColliderComponent, DestroyColliderComponent);
		pluginInterface->RegisterComponent<CapsuleColliderComponent>(SetupColliderComponent, DestroyColliderComponent);

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
