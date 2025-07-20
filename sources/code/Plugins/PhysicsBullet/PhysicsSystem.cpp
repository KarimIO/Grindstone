#include <bullet/btBulletDynamicsCommon.h>

#include "PhysicsSystem.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "PhysicsWorldContext.hpp"

using namespace Grindstone;
using namespace Grindstone::Physics;

static void SimulatePhysicsForObject(
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
	const btVector3& position = rigidBodyTransform.getOrigin();
	const btQuaternion& rotation = rigidBodyTransform.getRotation();

	transformComponent.position = { position.x(), position.y(), position.z() };
	transformComponent.rotation = { rotation.x(), rotation.y(), rotation.z(), rotation.w() };
}

namespace Grindstone {
	void PhysicsBulletSystem(entt::registry& registry) {
		Physics::WorldContext* cxt = Physics::WorldContext::GetActiveContext();
		if (cxt != nullptr) {
			btScalar dt = 1.0f / 30.0f;
			cxt->dynamicsWorld->stepSimulation(dt, 10);

			registry.view<Physics::RigidBodyComponent, TransformComponent>()
				.each(SimulatePhysicsForObject);
		}
	}
}
