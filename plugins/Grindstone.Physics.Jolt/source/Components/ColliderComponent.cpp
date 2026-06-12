#include <Jolt/Jolt.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <Common/Math.hpp>

#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>

using namespace JPH::literals;

using namespace Grindstone::Physics;
using namespace Grindstone::Math;

static float GetMinComponent(const glm::vec3& scale) {
	return glm::min(scale.x, scale.y, scale.z);
}

ColliderComponent* Grindstone::Physics::GetCollider(entt::registry& registry, entt::entity entityHandle) {
	SphereColliderComponent* sphere = registry.try_get<SphereColliderComponent>(entityHandle);
	if (sphere != nullptr) {
		return sphere;
	}

	BoxColliderComponent* box = registry.try_get<BoxColliderComponent>(entityHandle);
	if (box != nullptr) {
		return box;
	}

	PlaneColliderComponent* plane = registry.try_get<PlaneColliderComponent>(entityHandle);
	if (plane != nullptr) {
		return plane;
	}

	CapsuleColliderComponent* capsule = registry.try_get<CapsuleColliderComponent>(entityHandle);
	if (capsule != nullptr) {
		return capsule;
	}

	return nullptr;
}

REFLECT_STRUCT_BEGIN(SphereColliderComponent)
	REFLECT_STRUCT_MEMBER(radius)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

SphereColliderComponent SphereColliderComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	SphereColliderComponent sphere{};
	sphere.radius = radius;

	return sphere;
}

void SphereColliderComponent::Initialize(const TransformComponent& transformComponent) {
	collisionShape = new JPH::SphereShape(radius * GetMinComponent(transformComponent.scale), nullptr);
}

void SphereColliderComponent::SetRadius(float radius) {
	// TODO: Get scale
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

void PlaneColliderComponent::Initialize(const TransformComponent& transformComponent) {
}

void PlaneColliderComponent::SetCollider(Float3 planeNormal, float positionAlongNormal) {
	// TODO: Get scale
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

void BoxColliderComponent::Initialize(const TransformComponent& transformComponent) {
	// NOTE: Not sure if it's better to scale uniformly or not for boxes.
	const glm::vec3& scale = transformComponent.scale;
	glm::vec3 halfExtents(
		size.x * scale.x / 2.0f,
		size.y * scale.y / 2.0f,
		size.z * scale.z / 2.0f
	);

	float minComponent = GetMinComponent(halfExtents);
	float convexRadius = glm::min(minComponent / 2.0f, JPH::cDefaultConvexRadius);

	JPH::BoxShapeSettings boxShapeSettings(
		JPH::Vec3(
			halfExtents.x,
			halfExtents.y,
			halfExtents.z
		),
		convexRadius
	);
	collisionShape = boxShapeSettings.Create().Get();
}

void BoxColliderComponent::SetSize(Float3 size) {
	// TODO: Get scale
	JPH::BoxShapeSettings boxShapeSettings(
		JPH::Vec3(
			size.x / 2.0f,
			size.y / 2.0f,
			size.z / 2.0f
		)
	);
	collisionShape = boxShapeSettings.Create().Get();

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

void CapsuleColliderComponent::Initialize(const TransformComponent& transformComponent) {
	float scale = GetMinComponent(transformComponent.scale);
	collisionShape = new JPH::CapsuleShape(height * scale / 2.0f, radius * scale, nullptr);
}

void CapsuleColliderComponent::SetCollider(float radius, float height) {
	// TODO: Get scale
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
