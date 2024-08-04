#include <Common/Math.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "RigidBodyComponent.hpp"
#include "ColliderComponent.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(SphereColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void SphereColliderComponent::Initialize() {
	if (collisionShape) {
		AllocatorCore::Free(collisionShape);
	}

	collisionShape = AllocatorCore::Allocate<btSphereShape>(radius);
}

void SphereColliderComponent::SetRadius(float radius) {
	AllocatorCore::Free(collisionShape);
	collisionShape = AllocatorCore::Allocate<btSphereShape>(radius);

	this->radius = radius;
}

float SphereColliderComponent::GetRadius() const {
	return radius;
}

REFLECT_STRUCT_BEGIN(PlaneColliderComponent)
	REFLECT_STRUCT_MEMBER(planeNormal)
	REFLECT_STRUCT_MEMBER(positionAlongNormal)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

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

Float3 PlaneColliderComponent::GetPlaneNormal() const {
	return planeNormal;
}

float PlaneColliderComponent::GetPositionAlongNormal() const {
	return positionAlongNormal;
}

REFLECT_STRUCT_BEGIN(BoxColliderComponent)
	REFLECT_STRUCT_MEMBER(size)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void BoxColliderComponent::Initialize() {
	AllocatorCore::Free(collisionShape);

	collisionShape = AllocatorCore::Allocate<btBoxShape>(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);
}

void BoxColliderComponent::SetSize(Float3 size) {
	AllocatorCore::Free(collisionShape);

	collisionShape = AllocatorCore::Allocate<btBoxShape>(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);

	this->size = size;
}

Float3 BoxColliderComponent::GetSize() const {
	return size;
}

REFLECT_STRUCT_BEGIN(CapsuleColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_STRUCT_MEMBER(height)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void CapsuleColliderComponent::Initialize() {
	AllocatorCore::Free(collisionShape);

	collisionShape = AllocatorCore::Allocate<btCapsuleShape>(btCapsuleShape(radius, height));
}

void CapsuleColliderComponent::SetCollider(float radius, float height) {
	AllocatorCore::Free(collisionShape);
	collisionShape = AllocatorCore::Allocate<btCapsuleShape>(radius, height);

	this->radius = radius;
	this->height = height;
}

float CapsuleColliderComponent::GetRadius() const {
	return radius;
}

float CapsuleColliderComponent::GetHeight() const {
	return height;
}
