using System.Runtime.InteropServices;

namespace Grindstone {
	[StructLayout(LayoutKind.Sequential)]
	public struct TagComponent {
		public static string NativeComponentName => "Tag";

		#region Properties
		public string? Tag {
			get => Marshal.PtrToStringAnsi(TagComponentGetTag(ref this));
			set => TagComponentSetTag(ref this, value);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern System.IntPtr TagComponentGetTag(ref TagComponent comp);

		[DllImport("EngineCore")]
		private static extern void TagComponentSetTag(ref TagComponent comp, string? tag);
		#endregion
	}
}
