#include "PhysicsSystem.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"

namespace Grindstone {
	void PhysicsBulletSystem(entt::registry& registry) {
		auto broadphase = new btDbvtBroadphase();

		auto collisionConfiguration = new btDefaultCollisionConfiguration();
		auto dispatcher = new btCollisionDispatcher(collisionConfiguration);

		auto solver = new btSequentialImpulseConstraintSolver;

		auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

		dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

		/*btScalar mass = 4.0f;
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
		dynamicsWorld->addRigidBody(rbody);*/

		auto view = registry.view<const Physics::RigidBodyComponent, TransformComponent>();
		view.each(
			[&](
				const Physics::RigidBodyComponent& rigidBodyComponent,
				TransformComponent& transformComponent
			) {
				btScalar dt = 1.0f / 30.0f;
				dynamicsWorld->stepSimulation(dt, 10);
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
