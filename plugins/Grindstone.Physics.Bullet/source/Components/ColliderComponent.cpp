#include <Common/Math.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include <Grindstone.Physics.Bullet/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Bullet/include/Components/ColliderComponent.hpp>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

template<typename ComponentType>
void SetupColliderComponent(Grindstone::WorldContextSet& cxt, entt::entity entity) {
	entt::registry& registry = cxt.GetEntityRegistry();
	ComponentType& colliderComponent = registry.get<ComponentType>(entity);

	colliderComponent.Initialize();
	colliderComponent.collisionShape->setUserPointer(&colliderComponent);

	RigidBodyComponent* rigidBodyComponent = registry.try_get<RigidBodyComponent>(entity);
	Grindstone::TransformComponent* transformComponent = registry.try_get<Grindstone::TransformComponent>(entity);
	if (rigidBodyComponent != nullptr && transformComponent != nullptr) {
		SetupRigidBodyComponentWithCollider(
			cxt,
			rigidBodyComponent,
			transformComponent,
			&colliderComponent
		);
	}
}

REFLECT_STRUCT_BEGIN(SphereColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void SphereColliderComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entityId) {
	SetupColliderComponent<SphereColliderComponent>(cxtSet, entityId);
}

SphereColliderComponent SphereColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	SphereColliderComponent sphere{};
	sphere.radius = radius;

	return sphere;
}

void SphereColliderComponent::Initialize() {
	collisionShape = AllocatorCore::AllocateUnique<btSphereShape>(radius);
}

void SphereColliderComponent::SetRadius(float radius) {
	collisionShape = AllocatorCore::AllocateUnique<btSphereShape>(radius);

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

void PlaneColliderComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entityId) {
	SetupColliderComponent<PlaneColliderComponent>(cxtSet, entityId);
}

PlaneColliderComponent PlaneColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	PlaneColliderComponent plane{};
	plane.planeNormal = planeNormal;
	plane.positionAlongNormal = positionAlongNormal;

	return plane;
}

void PlaneColliderComponent::Initialize() {
	collisionShape = AllocatorCore::AllocateUnique<btStaticPlaneShape>(
		btVector3(
			planeNormal.x,
			planeNormal.y,
			planeNormal.z
		),
		positionAlongNormal
	);
}

void PlaneColliderComponent::SetCollider(Float3 planeNormal, float positionAlongNormal) {
	collisionShape = AllocatorCore::AllocateUnique<btStaticPlaneShape>(
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

void BoxColliderComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entityId) {
	SetupColliderComponent<BoxColliderComponent>(cxtSet, entityId);
}

BoxColliderComponent BoxColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	BoxColliderComponent box{};
	box.size = size;

	return box;
}

void BoxColliderComponent::Initialize() {
	collisionShape = AllocatorCore::AllocateUnique<btBoxShape>(
		btVector3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);
}

void BoxColliderComponent::SetSize(Float3 size) {
	collisionShape = AllocatorCore::AllocateUnique<btBoxShape>(
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

void CapsuleColliderComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entityId) {
	SetupColliderComponent<CapsuleColliderComponent>(cxtSet, entityId);
}

CapsuleColliderComponent CapsuleColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	CapsuleColliderComponent capsule{};
	capsule.radius = radius;
	capsule.height = height;

	return capsule;
}

void CapsuleColliderComponent::Initialize() {
	collisionShape = AllocatorCore::AllocateUnique<btCapsuleShape>(btCapsuleShape(radius, height));
}

void CapsuleColliderComponent::SetCollider(float radius, float height) {
	collisionShape = AllocatorCore::AllocateUnique<btCapsuleShape>(radius, height);

	this->radius = radius;
	this->height = height;
}

float CapsuleColliderComponent::GetRadius() const {
	return radius;
}

float CapsuleColliderComponent::GetHeight() const {
	return height;
}
