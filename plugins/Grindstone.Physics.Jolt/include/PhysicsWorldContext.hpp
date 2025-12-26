#pragma once

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <Common/HashedString.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <EngineCore/WorldContext/WorldContext.hpp>

const Grindstone::ConstHashedString physicsWorldContextName("PhysicsWorldContext");

namespace Grindstone::Physics {
	// Layer that objects can be in, determines which other objects it can collide with
	// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
	// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
	// but only if you do collision testing).
	namespace Layers {
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING = 1;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	};

	/// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
			switch (inObject1) {
			case Layers::NON_MOVING:
				return inObject2 == Layers::MOVING; // Non moving only collides with moving
			case Layers::MOVING:
				return true; // Moving collides with everything
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	namespace BroadPhaseLayers {
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr uint32_t NUM_LAYERS(2);
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
	public:
		BPLayerInterfaceImpl() {
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual uint32_t GetNumBroadPhaseLayers() const override {
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
			switch ((JPH::BroadPhaseLayer::Type)inLayer) {
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
			switch (inLayer1) {
			case Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// An example contact listener
	class MyContactListener : public JPH::ContactListener {
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override {
			std::cout << "Contact validate callback" << std::endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
			std::cout << "A contact was added" << std::endl;
		}

		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
		}

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override {
			std::cout << "A contact was removed" << std::endl;
		}
	};

	// An example activation listener
	class MyBodyActivationListener : public JPH::BodyActivationListener {
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override {
			std::cout << "A body got activated" << std::endl;
		}

		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override {
			std::cout << "A body went to sleep" << std::endl;
		}
	};

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

		BPLayerInterfaceImpl broadphaseLayerInterface;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter;
		ObjectLayerPairFilterImpl objectVsObjectLayerFilter;
		MyBodyActivationListener bodyActivationListener;
		MyContactListener contactListener;
	};
}
