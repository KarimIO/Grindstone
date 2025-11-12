#pragma once

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include <Common/HashedString.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <EngineCore/WorldContext/WorldContext.hpp>

const Grindstone::ConstHashedString physicsWorldContextName("PhysicsWorldContext");

namespace Grindstone::Physics {
	class WorldContext : public Grindstone::WorldContext {
	public:
		WorldContext();
		WorldContext(const WorldContext&) = delete;
		WorldContext(WorldContext&& ) noexcept = default;
		virtual ~WorldContext() override = default;

		JPH::BodyInterface& GetBodyInterface();
		JPH::PhysicsSystem& GetPhysicsSystem();
		JPH::TempAllocatorImpl& GetTempAllocator();
		JPH::JobSystemThreadPool& GetJobSystem();

		[[nodiscard]] static WorldContext* GetActiveContext();
		static void SetActiveContext(WorldContext& cxt);
		virtual void SetAsActive() override;

	protected:
		JPH::TempAllocatorImpl tempAllocator;
		JPH::JobSystemThreadPool jobSystem;
		JPH::PhysicsSystem physicsSystem;
		JPH::BodyInterface* bodyInterface = nullptr;
	};
}
