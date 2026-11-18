using Grindstone.Math;
using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable, StructLayout(LayoutKind.Explicit, Size = 632)]
	public struct TransformComponent {
		public static string NativeComponentName => "Transform";

		#region
		[FieldOffset(0)] Quaternion rotation;
		[FieldOffset(16)] Float3 position;
		[FieldOffset(28)] Float3 scale;
		#endregion

		public Quaternion LocalRotation { get => rotation; set => rotation = value; }
		public Float3 LocalPosition { get => position; set => position = value; }
		public Float3 Scale { get => scale; set => scale = value; }

		// TODO: Get World coordinates
		public Quaternion Rotation { get => rotation; set => rotation = value; }
		public Float3 Position { get => position; set => position = value; }

		public Float3 Forward
		{
			get => TransformComponentGetForward(ref this);
		}

		public Float3 Right
		{
			get => TransformComponentGetRight(ref this);
		}

		public Float3 Up
		{
			get => TransformComponentGetUp(ref this);
		}


		#region DllImports
		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetForward(ref TransformComponent comp);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetRight(ref TransformComponent comp);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetUp(ref TransformComponent comp);
		#endregion
	}
}
