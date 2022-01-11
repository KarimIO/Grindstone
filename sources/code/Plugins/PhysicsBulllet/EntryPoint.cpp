#include "pch.hpp"
#include <chrono>
#include <string>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <btBulletDynamicsCommon.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "Components/PhysicsWorldComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "PhysicsSystem.hpp"
using namespace Grindstone::Physics;

extern "C" {
	BULLET_PHYSICS_EXPORT void initializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->componentRegistrar->RegisterComponent<PhysicsWorldComponent>();

		pluginInterface->componentRegistrar->RegisterComponent<RigidBodyComponent>();
		
		pluginInterface->componentRegistrar->RegisterComponent<BoxColliderComponent>();
		pluginInterface->componentRegistrar->RegisterComponent<SphereColliderComponent>();
		pluginInterface->componentRegistrar->RegisterComponent<PlaneColliderComponent>();
		pluginInterface->componentRegistrar->RegisterComponent<CapsuleColliderComponent>();

		pluginInterface->systemRegistrar->RegisterSystem("PhysicsSystem", PhysicsBulletSystem);
		
		/*auto broadphase = new btDbvtBroadphase();

		auto collisionConfiguration = new btDefaultCollisionConfiguration();
		auto dispatcher = new btCollisionDispatcher(collisionConfiguration);

		auto solver = new btSequentialImpulseConstraintSolver;

		auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

		dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

		btScalar mass = 4.0f;
		btSphereShape* collisionShape = new btSphereShape(4.0f);
		btQuaternion quaternion;
		btVector3 position(0.0f, 2.0f, 0.0f);
		btTransform trans(quaternion, position);
		btDefaultMotionState* motionState = new btDefaultMotionState(trans);
		btRigidBody* rbody = new btRigidBody(
			mass,
			motionState,
			collisionShape
		);
		dynamicsWorld->addRigidBody(rbody);

		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		auto engineCore = pluginInterface->getEngineCore();
		while (true) {
			end = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsedSeconds = end - start;
			btScalar dt = elapsedSeconds.count();
			dynamicsWorld->stepSimulation(dt, 10);

			btTransform transform;
			rbody->getMotionState()->getWorldTransform(transform);
			btVector3 pos = transform.getOrigin();

			std::string str = std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ", " + std::to_string(pos.z());
			engineCore->Print(LogSeverity::Info, str.c_str());
		}*/
	}

	BULLET_PHYSICS_EXPORT void releaseModule(Plugins::Interface* pluginInterface) {
	}
}
