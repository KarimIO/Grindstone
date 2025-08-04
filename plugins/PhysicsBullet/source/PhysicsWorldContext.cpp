#include <bullet/btBulletDynamicsCommon.h>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <PhysicsBullet/include/PhysicsWorldContext.hpp>

using namespace Grindstone::Memory;

Grindstone::Physics::WorldContext* activeContext = nullptr;

Grindstone::Physics::WorldContext::WorldContext() {
	broadphase = AllocatorCore::AllocateUnique<btDbvtBroadphase>();
	collisionConfiguration = AllocatorCore::AllocateUnique<btDefaultCollisionConfiguration>();
	dispatcher = AllocatorCore::AllocateUnique<btCollisionDispatcher>(collisionConfiguration.Get());
	solver = AllocatorCore::AllocateUnique<btSequentialImpulseConstraintSolver>();
	dynamicsWorld = AllocatorCore::AllocateUnique<btDiscreteDynamicsWorld>(
		dispatcher.Get(),
		broadphase.Get(),
		solver.Get(),
		collisionConfiguration.Get()
	);

	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

Grindstone::Physics::WorldContext* Grindstone::Physics::WorldContext::GetActiveContext() {
	return activeContext;
}

void Grindstone::Physics::WorldContext::SetActiveContext(WorldContext& cxt) {
	activeContext = &cxt;
}

void Grindstone::Physics::WorldContext::SetAsActive() {
	activeContext = this;
}
