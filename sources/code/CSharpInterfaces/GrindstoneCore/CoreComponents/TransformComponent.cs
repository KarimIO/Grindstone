using Grindstone.Math;
using System.Runtime.InteropServices;

namespace Grindstone
{
	public class TransformComponent
	{
		System.IntPtr componentPtr;

		public TransformComponent(System.IntPtr componentPtr)
		{
			this.componentPtr = componentPtr;
		}

		public Quaternion Rotation
		{
			get => TransformComponentGetRotation(componentPtr);
			set => TransformComponentSetRotation(componentPtr, value);
		}

		public Float3 Position
		{
			get => TransformComponentGetPosition(componentPtr);
			set => TransformComponentSetPosition(componentPtr, value);
		}

		public Float3 Scale
		{
			get => TransformComponentGetScale(componentPtr);
			set => TransformComponentSetScale(componentPtr, value);
		}

		public Float3 Forward
		{
			get => TransformComponentGetForward(componentPtr);
		}

		public Float3 Right
		{
			get => TransformComponentGetRight(componentPtr);
		}

		public Float3 Up
		{
			get => TransformComponentGetUp(componentPtr);
		}


		#region DllImports
		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetForward(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetRight(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetUp(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern Quaternion TransformComponentGetRotation(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern void TransformComponentSetRotation(System.IntPtr comp, Quaternion rotation);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetPosition(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern void TransformComponentSetPosition(System.IntPtr comp, Float3 position);

		[DllImport("EngineCore")]
		static extern Float3 TransformComponentGetScale(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern void TransformComponentSetScale(System.IntPtr comp, Float3 scale);
		#endregion
	}
}
