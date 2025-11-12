#include <Jolt/Jolt.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <Common/Math.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>

using namespace JPH::literals;

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(SphereColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

SphereColliderComponent SphereColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	SphereColliderComponent sphere{};
	sphere.radius = radius;

	return sphere;
}

void SphereColliderComponent::Initialize() {
	collisionShape = new JPH::SphereShape(radius, nullptr);
}

void SphereColliderComponent::SetRadius(float radius) {
	collisionShape = new JPH::SphereShape(radius, nullptr);

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

PlaneColliderComponent PlaneColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	PlaneColliderComponent plane{};
	plane.planeNormal = planeNormal;
	plane.positionAlongNormal = positionAlongNormal;

	return plane;
}

void PlaneColliderComponent::Initialize() {
}

void PlaneColliderComponent::SetCollider(Float3 planeNormal, float positionAlongNormal) {
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

BoxColliderComponent BoxColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	BoxColliderComponent box{};
	box.size = size;

	return box;
}

void BoxColliderComponent::Initialize() {
	collisionShape = new JPH::BoxShape(
		JPH::Vec3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		),
		0.05f,
		nullptr
	);
}

void BoxColliderComponent::SetSize(Float3 size) {
	collisionShape = new JPH::BoxShape(
		JPH::Vec3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		),
		0.05f,
		nullptr
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

CapsuleColliderComponent CapsuleColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	CapsuleColliderComponent capsule{};
	capsule.radius = radius;
	capsule.height = height;

	return capsule;
}

void CapsuleColliderComponent::Initialize() {
	collisionShape = new JPH::CapsuleShape(height / 2.0f, radius, nullptr);
}

void CapsuleColliderComponent::SetCollider(float radius, float height) {
	collisionShape = new JPH::CapsuleShape(height / 2.0f, radius, nullptr);

	this->radius = radius;
	this->height = height;
}

float CapsuleColliderComponent::GetRadius() const {
	return radius;
}

float CapsuleColliderComponent::GetHeight() const {
	return height;
}
