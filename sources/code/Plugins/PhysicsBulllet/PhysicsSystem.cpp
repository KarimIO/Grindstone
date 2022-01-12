#include <bullet/btBulletDynamicsCommon.h>

#include "PhysicsSystem.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/PhysicsWorldComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::Physics;

btDynamicsWorld* currentDynamicsWorld = nullptr;

void InitializeCollider(ColliderComponent& colliderComponent) {
	if (colliderComponent.collisionShape == nullptr) {
		colliderComponent.Initialize();
		colliderComponent.collisionShape->setUserPointer(&colliderComponent);
	}
}

void SimulatePhysicsForObject(
	ColliderComponent& colliderComponent,
	RigidBodyComponent& rigidBodyComponent,
	TransformComponent& transformComponent
) {
	if (rigidBodyComponent.rigidBody == nullptr) {
		btScalar mass = rigidBodyComponent.GetMass();
		btQuaternion quaternion;
		btVector3 position(
			transformComponent.position.x,
			transformComponent.position.y,
			transformComponent.position.z
		);
		btTransform trans(quaternion, position);
		btDefaultMotionState* motionState = new btDefaultMotionState(trans);
		rigidBodyComponent.rigidBody = new btRigidBody(
			mass,
			motionState,
			colliderComponent.collisionShape
		);

		rigidBodyComponent.rigidBody->setUserPointer(&rigidBodyComponent);

		currentDynamicsWorld->addRigidBody(rigidBodyComponent.rigidBody);
	}
	else {
		btTransform rigidBodyTransform;
		auto motionState = rigidBodyComponent.rigidBody->getMotionState();
		motionState->getWorldTransform(rigidBodyTransform);
		auto position = rigidBodyTransform.getOrigin();
		auto rotation = rigidBodyTransform.getRotation();

		transformComponent.position = { position.x(), position.y(), position.z() };
		transformComponent.rotation = { rotation.x(), rotation.y(), rotation.z(), rotation.w() };
	}
}

namespace Grindstone {
	void PhysicsBulletSystem(entt::registry& registry) {
		Grindstone::Physics::PhysicsWorldComponent* physicsWorld = nullptr;
		auto physicsWorldView = registry.view<Grindstone::Physics::PhysicsWorldComponent>();
		physicsWorldView.each(
			[&](
				Grindstone::Physics::PhysicsWorldComponent& physicsWorldComponent
			) {
				physicsWorld = &physicsWorldComponent;
			}
		);

		if (physicsWorld == nullptr) {
			return;
		}

		if (physicsWorld->dynamicsWorld == nullptr) {
			physicsWorld->broadphase = new btDbvtBroadphase();
			physicsWorld->collisionConfiguration = new btDefaultCollisionConfiguration();
			physicsWorld->dispatcher = new btCollisionDispatcher(physicsWorld->collisionConfiguration);
			physicsWorld->solver = new btSequentialImpulseConstraintSolver();
			physicsWorld->dynamicsWorld = new btDiscreteDynamicsWorld(
				physicsWorld->dispatcher,
				physicsWorld->broadphase,
				physicsWorld->solver,
				physicsWorld->collisionConfiguration
			);

			physicsWorld->dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
		}

		currentDynamicsWorld = physicsWorld->dynamicsWorld;

		btScalar dt = 1.0f / 30.0f;
		currentDynamicsWorld->stepSimulation(dt, 10);

		registry.view<Physics::SphereColliderComponent>().each(InitializeCollider);
		registry.view<Physics::PlaneColliderComponent>().each(InitializeCollider);
		registry.view<Physics::CapsuleColliderComponent>().each(InitializeCollider);
		registry.view<Physics::BoxColliderComponent>().each(InitializeCollider);

		registry.view<Physics::SphereColliderComponent, Physics::RigidBodyComponent, TransformComponent>()
			.each(SimulatePhysicsForObject);

		registry.view<Physics::PlaneColliderComponent, Physics::RigidBodyComponent, TransformComponent>()
			.each(SimulatePhysicsForObject);

		registry.view<Physics::CapsuleColliderComponent, Physics::RigidBodyComponent, TransformComponent>()
			.each(SimulatePhysicsForObject);

		registry.view<Physics::BoxColliderComponent, Physics::RigidBodyComponent, TransformComponent>()
			.each(SimulatePhysicsForObject);

		currentDynamicsWorld = nullptr;
	}
}
