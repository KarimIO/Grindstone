#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"

struct btDbvtBroadphase;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace Grindstone {
	namespace Physics {
		struct PhysicsWorldComponent {
			btDbvtBroadphase* broadphase = nullptr;
			btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
			btCollisionDispatcher* dispatcher = nullptr;
			btSequentialImpulseConstraintSolver* solver = nullptr;
			btDiscreteDynamicsWorld* dynamicsWorld = nullptr;

			REFLECT("PhysicsWorld")
		};
	}
}
