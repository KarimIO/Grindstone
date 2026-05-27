using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public struct Quaternion {
			#region Public Fields
			public float x;
			public float y;
			public float z;
			public float w;
			#endregion

			#region Constructor Methods
			public Quaternion() {
				x = 0;
				y = 0;
				z = 0;
				w = 1;
			}

			public Quaternion(float x, float y, float z, float w) {
				this.x = x;
				this.y = y;
				this.z = z;
				this.w = w;
			}

			public Quaternion(EulerAngles angles)
			{
				float cy = (float)System.Math.Cos(angles.yaw * 0.5f);
				float sy = (float)System.Math.Sin(angles.yaw * 0.5f);
				float cr = (float)System.Math.Cos(angles.roll * 0.5f);
				float sr = (float)System.Math.Sin(angles.roll * 0.5f);
				float cp = (float)System.Math.Cos(angles.pitch * 0.5f);
				float sp = (float)System.Math.Sin(angles.pitch * 0.5f);

				x = sr * cp * cy - cr * sp * sy;
				y = cr * sp * cy + sr * cp * sy;
				z = cr * cp * sy - sr * sp * cy;
				w = cr * cp * cy + sr * sp * sy;
			}
			#endregion

			#region Public Methods
			public static Quaternion Identity => new(0.0f, 0.0f, 0.0f, 1.0f);
			public readonly EulerAngles ToEulerAngles() => new(this);
			public override readonly string ToString() => $"Quaternion({x}, {y}, {z}, {w})";

			public static implicit operator string(Quaternion q)
			{
				return q.ToString();
			}
			#endregion
		}
	}
}
