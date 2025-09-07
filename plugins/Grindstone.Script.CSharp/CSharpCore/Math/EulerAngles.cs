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

			#region Constants
			const float TWO_PI = (float)(2 * System.Math.PI);
			#endregion

			#region Constructor Methods
			public EulerAngles(float pitch, float yaw, float roll) {
				this.pitch = pitch % TWO_PI;
				this.yaw = yaw % TWO_PI;
				this.roll = roll % TWO_PI;
			}

			public EulerAngles(Float3 eulerAngles) {
				pitch = eulerAngles.x % TWO_PI;
				yaw = eulerAngles.y % TWO_PI;
				roll = eulerAngles.z % TWO_PI;
			}


			public static EulerAngles Zero => new EulerAngles(0.0f, 0.0f, 0.0f);
			#endregion

			#region Public Methods
			public override string ToString() => $"EulerAngles({pitch}rad, {yaw}rad, {roll}rad)";
			#endregion
		}
	}
}
