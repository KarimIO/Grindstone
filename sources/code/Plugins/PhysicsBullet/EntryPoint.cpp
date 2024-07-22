#include "pch.hpp"
#include <chrono>
#include <string>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <btBulletDynamicsCommon.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "PhysicsSystem.hpp"
using namespace Grindstone::Physics;

extern "C" {
	BULLET_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		pluginInterface->RegisterComponent<BoxColliderComponent>(SetupColliderComponent);
		pluginInterface->RegisterComponent<SphereColliderComponent>(SetupColliderComponent);
		pluginInterface->RegisterComponent<PlaneColliderComponent>(SetupColliderComponent);
		pluginInterface->RegisterComponent<CapsuleColliderComponent>(SetupColliderComponent);

		pluginInterface->RegisterComponent<RigidBodyComponent>(SetupRigidBodyComponent);

		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsBulletSystem);
	}

	BULLET_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
