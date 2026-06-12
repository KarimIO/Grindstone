#pragma once

#include <Common/Math.hpp>

namespace Grindstone::Ai {
	struct NavMeshLocomotionData {
		Math::Float3	desiredDirection;
		bool			isOnOffMeshLink;
		bool			hasReachedDestination;
		float			remainingDistance;
	};
}
