using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public class Quaternion {
			#region Public Fields
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
			protected float[] array = new float[4];
			#endregion

			#region Constructor Methods
			public Quaternion() {
				array[0] = 0;
				array[1] = 0;
				array[2] = 0;
				array[3] = 0;
			}

			public Quaternion(EulerAngles angles)
			{
				float cy = (float)System.Math.Cos(angles.yaw * 0.5f);
				float sy = (float)System.Math.Sin(angles.yaw * 0.5f);
				float cr = (float)System.Math.Cos(angles.roll * 0.5f);
				float sr = (float)System.Math.Sin(angles.roll * 0.5f);
				float cp = (float)System.Math.Cos(angles.pitch * 0.5f);
				float sp = (float)System.Math.Sin(angles.pitch * 0.5f);

				array[0] = sr * cp * cy - cr * sp * sy;
				array[1] = cr * sp * cy + sr * cp * sy;
				array[2] = cr * cp * sy - sr * sp * cy;
				array[3] = cr * cp * cy + sr * sp * sy;
			}
			#endregion

			#region Public Methods
			public override string ToString() => $"Quaternion({array[0]}, {array[1]}, {array[2]}, {array[3]})";
			#endregion
		}
	}
}
