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
			~Core();

			static Core& GetInstance();
			static Core* instance;

			btDbvtBroadphase* broadphase = nullptr;
			btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
			btCollisionDispatcher* dispatcher = nullptr;
			btSequentialImpulseConstraintSolver* solver = nullptr;
			btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
		};
	}
}
