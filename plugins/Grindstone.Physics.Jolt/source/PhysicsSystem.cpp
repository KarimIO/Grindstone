#include <Jolt/Jolt.h>

#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>

using namespace Grindstone;
using namespace Grindstone::Physics;

namespace Grindstone {
	void PhysicsJoltSystem(entt::registry& registry) {
		Physics::WorldContext* cxt = Physics::WorldContext::GetActiveContext();
		if (cxt != nullptr) {
			const float deltaTime = 1.0f / 30.0f;
			const int collisionSteps = 1;

			JPH::TempAllocator& tempAllocator = cxt->GetTempAllocator();
			JPH::JobSystem& jobSystem = cxt->GetJobSystem();
			cxt->GetPhysicsSystem().Update(deltaTime, collisionSteps, &tempAllocator, &jobSystem);

			registry.view<Physics::RigidBodyComponent, TransformComponent>()
				.each(
					[cxt](
						RigidBodyComponent& rigidBodyComponent,
						TransformComponent& transformComponent
					) {
						JPH::BodyID bodyId = rigidBodyComponent.GetBodyID();
						if (bodyId.IsInvalid()) {
							return;
						}

						JPH::RVec3 position = cxt->GetBodyInterface().GetCenterOfMassPosition(bodyId);
						JPH::Quat rotation = cxt->GetBodyInterface().GetRotation(bodyId);

						transformComponent.position = { position.GetX(), position.GetY(), position.GetZ() };
						transformComponent.rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW() };
				}
			);
		}
	}
}
