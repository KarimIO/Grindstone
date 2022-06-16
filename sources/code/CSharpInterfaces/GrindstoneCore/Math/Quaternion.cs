using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public class Quaternion {
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
			protected float[] array = new float[4];

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

				array[0] = cy * sr * cp - sy * cr * sp;
				array[1] = cy * cr * sp + sy * sr * cp;
				array[2] = sy * cr * cp - cy * sr * sp;
				array[3] = cy * cr * cp + sy * sr * sp;
			}

			public override string ToString() => $"Quaternion({array[0]}, {array[1]}, {array[2]}, {array[3]})";
		}
	}
}
