#include <iostream>
#include <stdarg.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

using namespace Grindstone::Memory;
using namespace JPH::literals;

Grindstone::Physics::WorldContext* activeContext = nullptr;

Grindstone::Physics::WorldContext::WorldContext() :
	tempAllocator(JPH::TempAllocatorImpl(10 * 1024 * 1024)),
	jobSystem(JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1))
{
	
	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint32_t cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint32_t cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint32_t cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint32_t cMaxContactConstraints = 1024;


	physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broadphaseLayerInterface, objectVsBroadphaseLayerFilter, objectVsObjectLayerFilter);

	physicsSystem.SetBodyActivationListener(&bodyActivationListener);
	physicsSystem.SetContactListener(&contactListener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	bodyInterface = &physicsSystem.GetBodyInterface();

	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	// physicsSystem.OptimizeBroadPhase();
}

JPH::PhysicsSystem& Grindstone::Physics::WorldContext::GetPhysicsSystem() {
	return physicsSystem;
}

JPH::BodyInterface& Grindstone::Physics::WorldContext::GetBodyInterface() {
	return *bodyInterface;
}

JPH::TempAllocatorImpl& Grindstone::Physics::WorldContext::GetTempAllocator() {
	return tempAllocator;
}

JPH::JobSystemThreadPool& Grindstone::Physics::WorldContext::GetJobSystem() {
	return jobSystem;
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
