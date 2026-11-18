using System.Drawing;
using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable, StructLayout(LayoutKind.Explicit, Size = 632)]
	public struct PointLightComponent {
		public static string NativeComponentName => "PointLight";

		#region Properties
		public Math.Float3 Color { get => color; set => color = value; }
		public float AttenuationRadius { get => attenuationRadius; set => attenuationRadius = value; }
		public float Intensity { get => intensity; set => intensity = value; }
		#endregion

		#region Private Fields
		[FieldOffset(480)] private Math.Float3 color;
		[FieldOffset(492)] private float attenuationRadius;
		[FieldOffset(496)] private float intensity;
		#endregion
	}
}
