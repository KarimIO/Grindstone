#include "RigidBodySystem.hpp"
#include "TransformSystem.hpp"
#include "ColliderSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"

RigidBodyComponent::RigidBodyComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_RIGID_BODY, object_handle, id) {}

RigidBodySystem::RigidBodySystem() : System(COMPONENT_RIGID_BODY)  {}

RigidBodySubSystem::RigidBodySubSystem(Space *space) : SubSystem(COMPONENT_RIGID_BODY, space) {
	broadphase_ = new btDbvtBroadphase();

	collision_configuration_ = new btDefaultCollisionConfiguration();
	dispatcher_ = new btCollisionDispatcher(collision_configuration_);

	solver_ = new btSequentialImpulseConstraintSolver;

	dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collision_configuration_);

	btScalar gravity = (btScalar)-9.81f;
	dynamics_world_->setGravity(btVector3(0, gravity, 0));
}

ComponentHandle RigidBodySubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

void RigidBodySystem::update(double dt) {
	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			// Step through the world
			RigidBodySubSystem *subsystem = (RigidBodySubSystem *)space->getSubsystem(system_type_);
			subsystem->dynamics_world_->stepSimulation((btScalar)dt, 10);

			for (auto &component : subsystem->components_) {
				GameObjectHandle game_object_id = component.game_object_handle_;
				auto &rigid_body = component.rigid_body_;

				ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
				TransformSubSystem *transform_sub = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));
				TransformComponent &transform_component = transform_sub->getComponent(transform_id);

				btTransform transform;
				rigid_body->getMotionState()->getWorldTransform(transform);
				btVector3 pos = transform.getOrigin();
				btVector3 vel = rigid_body->getLinearVelocity();

				transform_component.position_ = glm::vec3(pos.getX(), pos.getY(), pos.getZ());
				btQuaternion q = transform.getRotation();

				float ysqr = q.y() * q.y();

				float tpi = 3.14159f * 1.0f;

				// roll (x-axis rotation)
				float t0 = +2.0f * (q.w() * q.x() + q.y() * q.z());
				float t1 = +1.0f - 2.0f * (q.x() * q.x() + ysqr);
				transform_component.angles_.x = fmod(std::atan2(t0, t1), tpi);

				// pitch (y-axis rotation)
				float t2 = +2.0f * (q.w() * q.y() - q.z() * q.x());
				t2 = t2 > 1.0f ? 1.0f : t2;
				t2 = t2 < -1.0f ? -1.0f : t2;
				transform_component.angles_.y = fmod(std::asin(t2), tpi);

				// yaw (z-axis rotation)
				float t3 = +2.0f * (q.w() * q.z() + q.x() * q.y());
				float t4 = +1.0f - 2.0f * (ysqr + q.z() * q.z());
				transform_component.angles_.z = fmod(std::atan2(t3, t4), tpi);
			}
		}
	}
}

ComponentHandle RigidBodySubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void RigidBodySubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];
	auto object_handle = component.game_object_handle_;

	if (params.HasMember("mass")) {
		float mass = params["mass"].GetFloat();
		component.mass_ = mass;
	}

	ComponentHandle transform_id = space_->getObject(object_handle).getComponentHandle(COMPONENT_TRANSFORM);
	TransformSubSystem *transform_sub = (TransformSubSystem *)(space_->getSubsystem(COMPONENT_TRANSFORM));
	TransformComponent &transform_component = transform_sub->getComponent(transform_id);

	glm::vec3 posTrans = transform_component.position_;
	glm::vec3 angTrans = transform_component.angles_;

	btQuaternion quaternion(angTrans.y, angTrans.x, angTrans.z);
	btVector3 position(posTrans.x, posTrans.y, posTrans.z);

	btTransform trans(quaternion, position);

	ComponentHandle collider_handle = space_->getObject(object_handle).getComponentHandle(COMPONENT_COLLISION);
	ColliderSubSystem *subsystem = (ColliderSubSystem *)space_->getSubsystem(COMPONENT_COLLISION);
	btCollisionShape *shape = subsystem->getComponent(collider_handle).shape_;

	btDefaultMotionState* motionState = new btDefaultMotionState(trans);

	btVector3 inertia;
	shape->calculateLocalInertia(component.mass_, inertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(component.mass_, motionState, shape, inertia);
	component.rigid_body_ = new btRigidBody(rigidBodyCI);
	component.rigid_body_->setUserIndex(component.game_object_handle_);
	dynamics_world_->addRigidBody(component.rigid_body_);

	// this->->updateSingleAABB(component.rigid_body_);

	if (params.HasMember("restitution")) {
		float resitution = params["restitution"].GetFloat();
		component.restitution_ = resitution;
		component.rigid_body_->setRestitution(resitution);
	}

	if (params.HasMember("friction")) {
		float friction = params["friction"].GetFloat();
		component.friction_ = friction;
		component.rigid_body_->setFriction(friction);
	}

	if (params.HasMember("damping_linear")) {
		float damping_linear = params["damping_linear"].GetFloat();
		component.damping_linear_ = damping_linear;
	}

	if (params.HasMember("damping_rotational")) {
		float damping_rotational = params["damping_rotational"].GetFloat();
		component.damping_rotational_ = damping_rotational;
	}

	component.rigid_body_->setDamping(component.damping_linear_, component.damping_rotational_);
}

void RigidBodyComponent::setMass(float mass) {
	rigid_body_->setMassProps(mass, btVector3(0, 0, 0));
}

void RigidBodyComponent::setIntertia(float x, float y, float z) {
	/*btVector3 fallInertia(x, y, z);
	if (mass_ != 0.0f)
		shape->calculateLocalInertia(mass, fallInertia);*/
}

void RigidBodyComponent::setFriction(float f) {
	rigid_body_->setFriction(f);
}

void RigidBodyComponent::setRestitution(float r) {
	rigid_body_->setRestitution(r);
}

void RigidBodyComponent::setDamping(float linear, float rotational) {
	rigid_body_->setDamping(linear, rotational);
}

void RigidBodyComponent::applyForce(glm::vec3 pos, glm::vec3 force) {
	rigid_body_->applyForce(btVector3(pos.x, pos.y, pos.z), btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::applyCentralForce(glm::vec3 force) {
	rigid_body_->applyCentralForce(btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::applyImpulse(glm::vec3 pos, glm::vec3 force) {
	rigid_body_->applyForce(btVector3(pos.x, pos.y, pos.z), btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::applyCentralImpulse(glm::vec3 force) {
	rigid_body_->applyCentralForce(btVector3(force.x, force.y, force.z));
}

RigidBodyComponent & RigidBodySubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * RigidBodySubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RigidBodySubSystem::getNumComponents() {
	return components_.size();
}

void RigidBodySubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	RigidBodyComponent &c = getComponent(handle);

	w.Key("restitution");
	w.Double(c.restitution_);

	w.Key("friction");
	w.Double(c.friction_);

	w.Key("damping_linear");
	w.Double(c.damping_linear_);

	w.Key("damping_rotational");
	w.Double(c.damping_rotational_);
}

void RigidBodySubSystem::removeComponent(ComponentHandle handle) {
	components_.erase(components_.begin() + handle);
}

RigidBodySubSystem::~RigidBodySubSystem() {
	for (int i = 0; i < components_.size(); i++) {
		dynamics_world_->removeRigidBody(components_[i].rigid_body_);
		delete components_[i].rigid_body_;
	}

	components_.clear();
	delete dynamics_world_;
	delete solver_;
	delete collision_configuration_;
	delete dispatcher_;
	delete broadphase_;
}
