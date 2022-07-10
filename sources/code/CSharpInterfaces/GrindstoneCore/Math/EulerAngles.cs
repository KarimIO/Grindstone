using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public struct EulerAngles {
			#region Public Fields
			public float pitch;
			public float yaw;
			public float roll;
			#endregion

			#region Public Methods
			public override string ToString() => $"EulerAngles({pitch}rad, {yaw}rad, {roll}rad)";
			#endregion
		}
	}
}
