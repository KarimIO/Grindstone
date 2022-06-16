#include "Common/Math.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "ColliderComponent.hpp"
#include "RigidBodyComponent.hpp"
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(SphereColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::Physics::SetupColliderComponent(ECS::Entity& entity, void* componentPtr) {
	auto colliderComponent = (ColliderComponent *)componentPtr;

	if (colliderComponent->collisionShape == nullptr) {
		colliderComponent->Initialize();
		colliderComponent->collisionShape->setUserPointer(colliderComponent);
	}

	auto& registry = entity.GetSceneEntityRegistry();
	auto entityHandle = entity.GetHandle();
	auto rigidBodyComponent = registry.try_get<RigidBodyComponent>(entityHandle);
	auto transformComponent = registry.try_get<TransformComponent>(entityHandle);
	if (rigidBodyComponent && transformComponent) {
		SetupRigidBodyComponentWithCollider(
			rigidBodyComponent,
			transformComponent,
			colliderComponent
		);
	}
}

SphereColliderComponent::SphereColliderComponent(float radius)
	: radius(radius) {
	collisionShape = new btSphereShape(radius);
}

void SphereColliderComponent::Initialize() {
	if (collisionShape) {
		delete collisionShape;
	}

	collisionShape = new btSphereShape(radius);
}

void SphereColliderComponent::SetRadius(float radius) {
	delete collisionShape;
	collisionShape = new btSphereShape(radius);

	this->radius = radius;
}

float SphereColliderComponent::GetRadius() {
	return radius;
}

REFLECT_STRUCT_BEGIN(PlaneColliderComponent)
	REFLECT_STRUCT_MEMBER(planeNormal)
	REFLECT_STRUCT_MEMBER(positionAlongNormal)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

PlaneColliderComponent::PlaneColliderComponent(Float3 planeNormal, float positionAlongNormal)
	: planeNormal(planeNormal), positionAlongNormal(positionAlongNormal) {
	collisionShape = new btStaticPlaneShape(
		btVector3(
			planeNormal.x,
			planeNormal.y,
			planeNormal.z
		),
		positionAlongNormal
	);
}

void PlaneColliderComponent::Initialize() {
	if (collisionShape) {
		delete collisionShape;
	}

	collisionShape = new btStaticPlaneShape(
		btVector3(
			planeNormal.x,
			planeNormal.y,
			planeNormal.z
		),
		positionAlongNormal
	);
}

void PlaneColliderComponent::SetCollider(Float3 planeNormal, float positionAlongNormal) {
	delete collisionShape;
	collisionShape = new btStaticPlaneShape(
		btVector3(
			planeNormal.x,
			planeNormal.y,
			planeNormal.z
		),
		positionAlongNormal
	);

	this->planeNormal = planeNormal;
	this->positionAlongNormal = positionAlongNormal;
}

Float3 PlaneColliderComponent::GetPlaneNormal() {
	return planeNormal;
}

float PlaneColliderComponent::GetPositionAlongNormal() {
	return positionAlongNormal;
}

REFLECT_STRUCT_BEGIN(BoxColliderComponent)
	REFLECT_STRUCT_MEMBER(size)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

BoxColliderComponent::BoxColliderComponent(Float3 size)
	: size(size) {
	collisionShape = new btBoxShape(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);
}

void BoxColliderComponent::Initialize() {
	if (collisionShape) {
		delete collisionShape;
	}

	collisionShape = new btBoxShape(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);
}

void BoxColliderComponent::SetSize(Float3 size) {
	delete collisionShape;
	collisionShape = new btBoxShape(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);

	this->size = size;
}

Float3 BoxColliderComponent::GetSize() {
	return size;
}

REFLECT_STRUCT_BEGIN(CapsuleColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_STRUCT_MEMBER(height)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

CapsuleColliderComponent::CapsuleColliderComponent(float radius, float height)
	: radius(radius), height(height) {
	collisionShape = new btCapsuleShape(radius, height);
}

void CapsuleColliderComponent::Initialize() {
	if (collisionShape) {
		delete collisionShape;
	}

	collisionShape = new btCapsuleShape(radius, height);
}

void CapsuleColliderComponent::SetCollider(float radius, float height) {
	delete collisionShape;
	collisionShape = new btCapsuleShape(radius, height);

	this->radius = radius;
	this->height = height;
}

float CapsuleColliderComponent::GetRadius() {
	return radius;
}

float CapsuleColliderComponent::GetHeight() {
	return height;
}
