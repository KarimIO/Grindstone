#include <Jolt/Jolt.h>

#include <EngineCore/Logger.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>

using namespace Grindstone;
using namespace Grindstone::Physics;

namespace Grindstone {
	void PhysicsJoltSystem(Grindstone::WorldContextSet& worldContextSet) {
		Physics::WorldContext* cxt = static_cast<Physics::WorldContext*>(worldContextSet.GetContext(physicsWorldContextName));
		if (cxt != nullptr) {
			const float deltaTime = 1.0f / 30.0f;
			const int collisionSteps = 1;

			JPH::TempAllocator& tempAllocator = cxt->GetTempAllocator();
			JPH::JobSystem& jobSystem = cxt->GetJobSystem();
			cxt->GetPhysicsSystem().Update(deltaTime, collisionSteps, &tempAllocator, &jobSystem);

			entt::registry& registry = worldContextSet.GetEntityRegistry();
			auto view = registry.view<Grindstone::Physics::RigidBodyComponent, Grindstone::TransformComponent>();

			view.each(
					[&cxt](
						Grindstone::Physics::RigidBodyComponent& rigidBodyComponent,
						Grindstone::TransformComponent& transformComponent
					) {
						JPH::BodyID bodyId = rigidBodyComponent.GetBodyID();
						if (bodyId.IsInvalid()) {
							return;
						}

						JPH::RVec3 position = cxt->GetBodyInterface().GetCenterOfMassPosition(bodyId);
						JPH::Quat rotation = cxt->GetBodyInterface().GetRotation(bodyId);

						transformComponent.position = Grindstone::Math::Float3( position.GetX(), position.GetY(), position.GetZ() );
						GPRINT_INFO_V(Grindstone::LogSource::Physics, "Position: ({}, {}, {})", position.GetX(), position.GetY(), position.GetZ());
						transformComponent.rotation = Grindstone::Math::Quaternion( rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ() );
				}
			);
		}
	}
}
