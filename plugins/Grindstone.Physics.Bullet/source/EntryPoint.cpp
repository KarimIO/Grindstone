#include <Grindstone.Physics.Bullet/include/pch.hpp>
#include <chrono>
#include <string>

#include <btBulletDynamicsCommon.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Physics.Bullet/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Bullet/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Bullet/include/PhysicsSystem.hpp>
#include <Grindstone.Physics.Bullet/include/PhysicsWorldContext.hpp>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

extern "C" {
	BULLET_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		pluginInterface->RegisterComponent<BoxColliderComponent>();
		pluginInterface->RegisterComponent<SphereColliderComponent>();
		pluginInterface->RegisterComponent<PlaneColliderComponent>();
		pluginInterface->RegisterComponent<CapsuleColliderComponent>();

		pluginInterface->RegisterWorldContextFactory<Grindstone::Physics::WorldContext>(physicsWorldContextName);
		pluginInterface->RegisterComponent<RigidBodyComponent>();
		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsBulletSystem);
	}

	BULLET_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterSystem("PhysicsSystem");
		pluginInterface->UnregisterComponent<RigidBodyComponent>();
		pluginInterface->UnregisterWorldContextFactory(physicsWorldContextName);

		pluginInterface->UnregisterComponent<CapsuleColliderComponent>();
		pluginInterface->UnregisterComponent<PlaneColliderComponent>();
		pluginInterface->UnregisterComponent<SphereColliderComponent>();
		pluginInterface->UnregisterComponent<BoxColliderComponent>();
	}
}
