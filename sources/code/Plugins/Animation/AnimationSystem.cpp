#include <iostream>
#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AnimationSystem.hpp"
#include "EngineCore/CoreComponents/AnimatorComponent/AnimatorComponent.hpp"
#include "EngineCore/Assets/Rig/Rig.hpp"
#include "EngineCore/EngineCore.hpp"

void AnimationSystem(entt::registry& registry) {
		auto view = registry.view<Grindstone::AnimatorComponent>();
		
		view.each(
			[&](
				Grindstone::AnimatorComponent& animatorComponent
			) {
				Grindstone::Rig* rig = nullptr;
				animatorComponent.time += 1.0f / 60.0f;
				animatorComponent.animationClip->GetFrameMatrices(
					animatorComponent.time,
					rig,
					animatorComponent.boneTransformations
				);
			}
		);
}
