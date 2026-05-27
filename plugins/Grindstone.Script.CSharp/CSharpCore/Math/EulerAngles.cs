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

			public EulerAngles(Quaternion q)
			{
				// roll (x-axis rotation)
				double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
				double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
				roll = (float)System.Math.Atan2(sinr_cosp, cosr_cosp);

				// pitch (y-axis rotation)
				double sinp = 2 * (q.w * q.y - q.z * q.x);
				if (System.Math.Abs(sinp) >= 1)
				{
					pitch = (float)System.Math.CopySign(System.Math.PI / 2, sinp);
				}
				else
				{
					pitch = (float)System.Math.Asin(sinp);
				}

				// yaw (z-axis rotation)
				double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
				double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
				yaw = (float)System.Math.Atan2(siny_cosp, cosy_cosp);
			}

			public static EulerAngles Zero => new(0.0f, 0.0f, 0.0f);
			#endregion

			#region Public Methods
			public Quaternion ToQuaternion() => new(this);
			public override string ToString() => $"EulerAngles({pitch}rad, {yaw}rad, {roll}rad)";
			#endregion
		}
	}
}
