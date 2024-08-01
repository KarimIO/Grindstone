#include <bullet/btBulletDynamicsCommon.h>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "Core.hpp"

using namespace Grindstone::Physics;
using namespace Grindstone::Memory;

Grindstone::Physics::Core* Grindstone::Physics::Core::instance = nullptr;

Core::Core() {
	broadphase = AllocatorCore::Allocate<btDbvtBroadphase>();
	collisionConfiguration = AllocatorCore::Allocate<btDefaultCollisionConfiguration>();
	dispatcher = AllocatorCore::Allocate<btCollisionDispatcher>(collisionConfiguration);
	solver = AllocatorCore::Allocate<btSequentialImpulseConstraintSolver>();
	dynamicsWorld = AllocatorCore::Allocate<btDiscreteDynamicsWorld>(
		dispatcher,
		broadphase,
		solver,
		collisionConfiguration
	);

	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

	instance = this;
}

Core::~Core() {
	AllocatorCore::Free(dynamicsWorld);
	AllocatorCore::Free(solver);
	AllocatorCore::Free(dispatcher);
	AllocatorCore::Free(collisionConfiguration);
	AllocatorCore::Free(broadphase);

	instance = nullptr;
}

Core& Core::GetInstance() {
	return *instance;
}
