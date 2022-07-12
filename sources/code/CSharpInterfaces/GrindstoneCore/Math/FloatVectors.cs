using System.Runtime.InteropServices;

namespace Grindstone.Math {
	[System.Serializable, StructLayout(LayoutKind.Sequential)]
	public struct Float2 {
		#region Public Fields
		public float x;
		public float y;
		#endregion

		#region Constructor Methods
		public Float2(float value) {
			x = y = value;
		}

		public Float2(float x, float y) {
			this.x = x;
			this.y = y;
		}
		#endregion

		#region Operator Methods
		public static Float2 operator +(Float2 a) => a;
		public static Float2 operator -(Float2 a) {
			return new Float2(
				-a.x,
				-a.y
			);
		}

		public static Float2 operator +(Float2 a, Float2 b) {
			return new Float2(
				a.x + b.x,
				a.y + b.y
			);
		}

		public static Float2 operator -(Float2 a, Float2 b) {
			return new Float2(
				a.x - b.x,
				a.y - b.y
			);
		}

		public static Float2 operator *(Float2 a, float b) {
			return new Float2(
				a.x * b,
				a.y * b);
		}

		public static Float2 operator /(Float2 a, float b) {
			if (b == 0) {
				throw new System.DivideByZeroException();
			}

			return new Float2(
				a.x / b,
				a.y / b
			);
		}
		#endregion

		#region Public Methods
		public double GetMagnitude() {
			return System.Math.Sqrt(
				x * x +
				y * y
			);
		}

		public float Dot(Float2 other) {
			return
				x * other.x +
				y * other.y;
		}

		public Float2 Cross(Float2 other) {
			return new Float2(
				y * other.x - x * other.x,
				x * other.y - y * other.x
			);
		}

		public override string ToString() => $"Float2({x}, {y})";
		#endregion
	}

	[System.Serializable, StructLayout(LayoutKind.Sequential)]
	public struct Float3 {
		#region Public Fields
		public float x;
		public float y;
		public float z;
		#endregion

		#region Constructor Methods
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
		#endregion

		#region Operator Methods
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
		#endregion

		#region Public Methods
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
		#endregion
	}

	[System.Serializable, StructLayout(LayoutKind.Sequential)]
	public struct Float4 {
		#region Public Fields
		public float x;
		public float y;
		public float z;
		public float w;
		#endregion

		#region Constructor Methods
		public Float4(float value) {
			x = y = z = w = value;
		}

		public Float4(float x, float y) {
			this.x = x;
			this.y = y;
			z = 0;
			w = 0;
		}

		public Float4(float x, float y, float z) {
			this.x = x;
			this.y = y;
			this.z = z;
			w = 0;
		}

		public Float4(float x, float y, float z, float w) {
			this.x = x;
			this.y = y;
			this.z = z;
			this.w = w;
		}
		#endregion

		#region Operator Methods
		public static Float4 operator +(Float4 a) => a;
		public static Float4 operator -(Float4 a) {
			return new Float4(
				-a.x,
				-a.y,
				-a.z,
				-a.w
			);
		}

		public static Float4 operator +(Float4 a, Float4 b) {
			return new Float4(
				a.x + b.x,
				a.y + b.y,
				a.z + b.z,
				a.w + b.w
			);
		}

		public static Float4 operator -(Float4 a, Float4 b) {
			return new Float4(
				a.x - b.x,
				a.y - b.y,
				a.z - b.z,
				a.w - b.w
			);
		}

		public static Float4 operator *(Float4 a, float b) {
			return new Float4(
				a.x * b,
				a.y * b,
				a.z * b,
				a.w * b);
		}

		public static Float4 operator /(Float4 a, float b) {
			if (b == 0) {
				throw new System.DivideByZeroException();
			}

			return new Float4(
				a.x / b,
				a.y / b,
				a.z / b,
				a.w / b
			);
		}
		#endregion

		#region Public Methods
		public double GetMagnitude() {
			return System.Math.Sqrt(
				x * x +
				y * y +
				z * z +
				w * w
			);
		}

		public float Dot(Float4 other) {
			return
				x * other.x +
				y * other.y +
				z * other.z +
				w * other.w;
		}

		public override string ToString() => $"Float4({x}, {y}, {z}, {w})";
		#endregion
	}
}
