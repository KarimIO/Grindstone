using System.Runtime.InteropServices;

namespace Grindstone {
	namespace Math {
		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public struct Float3 {
			public float x;
			public float y;
			public float z;

			public Float3(float value) {
				x = y = z = value;
			}

			public Float3(float x, float y) {
				this.x = x;
				this.y = y;
				z = 0;
			}

			public Float3(float x, float y, float z) {
				this.x = x;
				this.y = y;
				this.z = z;
			}

			public static Float3 operator +(Float3 a) => a;
			public static Float3 operator -(Float3 a) {
				return new Float3(
					-a.x,
					-a.y,
					-a.z
				);
			}

			public static Float3 operator +(Float3 a, Float3 b) {
				return new Float3(
					a.x + b.x,
					a.y + b.y,
					a.z + b.z
				);
			}

			public static Float3 operator -(Float3 a, Float3 b) {
				return new Float3(
					a.x - b.x,
					a.y - b.y,
					a.z - b.z
				);
			}

			public static Float3 operator *(Float3 a, float b) {
				return new Float3(
					a.x * b,
					a.y * b,
					a.z * b);
			}

			public static Float3 operator /(Float3 a, float b) {
				if (b == 0) {
					throw new System.DivideByZeroException();
				}

				return new Float3(
					a.x / b,
					a.y / b,
					a.z / b
				);
			}

			public double GetMagnitude() {
				return System.Math.Sqrt(
					x * x +
					y * y +
					z * z
				);
			}

			public float Dot(Float3 other) {
				return
					x * other.x +
					y * other.y +
					z * other.z;
			}

			public Float3 Cross(Float3 other) {
				return new Float3(
					y * other.z - z * other.y,
					z * other.x - x * other.z,
					x * other.y - y * other.x
				);
			}

			public override string ToString() => $"Float3({x}, {y}, {z})";
		}
	}
}
