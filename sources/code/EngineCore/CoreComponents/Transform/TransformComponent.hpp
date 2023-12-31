#pragma once

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>

namespace Grindstone {
	struct TransformComponent {
		Math::Quaternion rotation;
		Math::Float3 position = Math::Float3(0.0f, 0.0f, 0.0f);
		Math::Float3 scale = Math::Float3(1.f, 1.f, 1.f);

		static Math::Matrix4 GetWorldTransformMatrix(ECS::Entity entity) {
			return GetWorldTransformMatrix(entity.GetHandle(), entity.GetSceneEntityRegistry());
		}

		static Math::Matrix4 GetWorldTransformMatrix(entt::entity entity, entt::registry& registry) {
			Math::Matrix4 matrix = Math::Matrix4(1.0f);
			entt::entity currentEntity = entity;
			while (currentEntity != entt::null) {
				TransformComponent& transformComp = registry.get<TransformComponent>(currentEntity);

				matrix = transformComp.GetTransformMatrix() * matrix;

				ParentComponent& parentComp = registry.get<ParentComponent>(currentEntity);
				currentEntity = parentComp.parentEntity;
			}

			return matrix;
		}

		static Math::Float3 GetWorldPosition(ECS::Entity entity) {
			return GetWorldPosition(entity.GetHandle(), entity.GetSceneEntityRegistry());
		}

		static Math::Float3 GetWorldPosition(entt::entity entity, entt::registry& registry) {
			Math::Matrix4 worldMatrix = GetWorldTransformMatrix(entity, registry);

			return Math::Float3(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]);
		}

		void SetLocalMatrix(Math::Matrix4 localMatrix) {
			rotation = Math::Quaternion(localMatrix);

			for (int i = 0; i < 3; i++) {
				position[i] = localMatrix[3][i];

				scale[i] = glm::sqrt(
					localMatrix[i][0] * localMatrix[i][0]
					+ localMatrix[i][1] * localMatrix[i][1]
					+ localMatrix[i][2] * localMatrix[i][2]);
			}
		}

		void SetWorldMatrixRelativeTo(Math::Matrix4 newWorldMatrix, Math::Matrix4 parentMatrix) {
			Math::Matrix4 localMatrix = glm::inverse(parentMatrix) * newWorldMatrix;
			SetLocalMatrix(localMatrix);
		}

		Math::Matrix4 GetTransformMatrix() const {
			return glm::translate(position) *
				glm::toMat4(rotation) *
				glm::scale(scale);
		}

		Math::Float3 GetForward() const {
			return rotation * Math::Float3(0.0f, 0.0f, 1.0f);
		}

		Math::Float3 GetRight() const {
			return rotation * Math::Float3(1.0f, 0.0f, 0.0f);
		}

		Math::Float3 GetUp() const {
			return rotation * Math::Float3(0.0f, 1.0f, 0.0f);
		}

		REFLECT("Transform")
	};
}
