#pragma once

struct btDbvtBroadphase;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace Grindstone {
	namespace Physics {
		class Core {
		public:
			Core();
			static Core& GetInstance();

			btDbvtBroadphase* broadphase = nullptr;
			btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
			btCollisionDispatcher* dispatcher = nullptr;
			btSequentialImpulseConstraintSolver* solver = nullptr;
			btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
		};
	}
}
