#include <bullet/btBulletDynamicsCommon.h>
#include "Core.hpp"
using namespace Grindstone::Physics;

Core::Core() {
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(
		dispatcher,
		broadphase,
		solver,
		collisionConfiguration
	);

	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

Core& Core::GetInstance() {
	static Core instance;
	return instance;
}
