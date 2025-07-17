#pragma once

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

		Grindstone::UniquePtr<struct btDbvtBroadphase> broadphase = nullptr;
		Grindstone::UniquePtr<class btDefaultCollisionConfiguration> collisionConfiguration = nullptr;
		Grindstone::UniquePtr<class btCollisionDispatcher> dispatcher = nullptr;
		Grindstone::UniquePtr<class btSequentialImpulseConstraintSolver> solver = nullptr;
		Grindstone::UniquePtr<class btDiscreteDynamicsWorld> dynamicsWorld = nullptr;

		[[nodiscard]] static WorldContext* GetActiveContext();
		static void SetActiveContext(WorldContext& cxt);
		virtual void SetAsActive() override;
	};
}
