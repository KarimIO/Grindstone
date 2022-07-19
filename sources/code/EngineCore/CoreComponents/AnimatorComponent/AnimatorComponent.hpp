#pragma once

#include <string>
#include <glm/glm.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/AnimationClip/AnimationClip.hpp"

namespace Grindstone {
	struct AnimatorComponent {
		std::vector<glm::mat4> boneTransformations;
		AnimationClip* animationClip;
		float time;

		REFLECT("Animator")
	};
}
