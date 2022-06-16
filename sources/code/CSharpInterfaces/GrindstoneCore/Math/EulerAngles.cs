using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public struct EulerAngles {
			public float pitch;
			public float yaw;
			public float roll;

			public override string ToString() => $"EulerAngles({pitch}rad, {yaw}rad, {roll}rad)";
		}
	}
}
