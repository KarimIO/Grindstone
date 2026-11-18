using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable, StructLayout(LayoutKind.Explicit, Size = 20)]
	public struct CameraComponent {
		public static string NativeComponentName => "Camera";

		[FieldOffset(0)] private byte isMainCamera;
		[FieldOffset(1)] private byte isOrthographic;
		[FieldOffset(4)] private float nearPlaneDistance;
		[FieldOffset(8)] private float farPlaneDistance;
		[FieldOffset(12)] private float fieldOfView;
		[FieldOffset(16)] private float aspectRatio;

	}
}
