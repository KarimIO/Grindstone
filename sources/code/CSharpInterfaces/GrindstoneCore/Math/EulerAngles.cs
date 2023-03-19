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

			#region Constructor Methods
			public EulerAngles(float pitch, float yaw, float roll) {
				this.pitch = pitch;
				this.yaw = yaw;
				this.roll = roll;
			}

			public EulerAngles(Float3 eulerAngles) {
				pitch = eulerAngles.x;
				yaw = eulerAngles.y;
				roll = eulerAngles.z;
			}
			#endregion

			#region Public Methods
			public override string ToString() => $"EulerAngles({pitch}rad, {yaw}rad, {roll}rad)";
			#endregion
		}
	}
}
