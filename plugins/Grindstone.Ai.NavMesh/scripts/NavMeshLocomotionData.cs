using System.Runtime.InteropServices;

namespace Grindstone.Ai {
	[System.Serializable, StructLayout(LayoutKind.Sequential)]
	public struct NavMeshLocomotionData {
		Grindstone.Math.Float3	desiredDirection;
		bool			isOnOffMeshLink;
		bool			hasReachedDestination;
		float			remainingDistance;
	}
}
