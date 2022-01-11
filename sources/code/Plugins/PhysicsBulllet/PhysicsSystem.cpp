#include <bullet/btBulletDynamicsCommon.h>

#include "PhysicsSystem.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/PhysicsWorldComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"

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

		/**/

		btScalar dt = 1.0f / 30.0f;
		physicsWorld->dynamicsWorld->stepSimulation(dt, 10);

		auto view = registry.view<Physics::SphereColliderComponent, Physics::RigidBodyComponent, TransformComponent>();
		view.each(
			[&](
				Physics::ColliderComponent& colliderComponent,
				Physics::RigidBodyComponent& rigidBodyComponent,
				TransformComponent& transformComponent
			) {
				if (colliderComponent.collisionShape == nullptr) {
					colliderComponent.Initialize();
				}

				if (rigidBodyComponent.rigidBody == nullptr) {
					btScalar mass = 4.0f;
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

					physicsWorld->dynamicsWorld->addRigidBody(rigidBodyComponent.rigidBody);
				}

				btTransform rigidBodyTransform;
				auto motionState = rigidBodyComponent.rigidBody->getMotionState();
				motionState->getWorldTransform(rigidBodyTransform);
				auto position = rigidBodyTransform.getOrigin();
				auto rotation = rigidBodyTransform.getRotation();

				transformComponent.position = { position.x(), position.y(), position.z() };
				transformComponent.rotation = { rotation.x(), rotation.y(), rotation.z(), rotation.w() };
			}
		);
	}
}
