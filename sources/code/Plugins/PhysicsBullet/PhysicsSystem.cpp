#include <bullet/btBulletDynamicsCommon.h>

#include "PhysicsSystem.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Core.hpp"

using namespace Grindstone;
using namespace Grindstone::Physics;

void SimulatePhysicsForObject(
	RigidBodyComponent& rigidBodyComponent,
	TransformComponent& transformComponent
) {
	if (rigidBodyComponent.rigidBody == nullptr) {
		return;
	}

	btTransform rigidBodyTransform;
	auto motionState = rigidBodyComponent.rigidBody->getMotionState();
	if (motionState == nullptr) {
		return;
	}

	motionState->getWorldTransform(rigidBodyTransform);
	auto position = rigidBodyTransform.getOrigin();
	auto rotation = rigidBodyTransform.getRotation();

	transformComponent.position = { position.x(), position.y(), position.z() };
	transformComponent.rotation = { rotation.x(), rotation.y(), rotation.z(), rotation.w() };
}

namespace Grindstone {
	void PhysicsBulletSystem(entt::registry& registry) {
		Core& core = Core::GetInstance();

		btScalar dt = 1.0f / 30.0f;
		core.dynamicsWorld->stepSimulation(dt, 10);

		registry.view<Physics::RigidBodyComponent, TransformComponent>()
			.each(SimulatePhysicsForObject);
	}
}
