using System.Runtime.InteropServices;

namespace Grindstone {
	public class TagComponent {
		System.IntPtr componentPtr;

		public TagComponent(System.IntPtr componentPtr)
		{
			this.componentPtr = componentPtr;
		}

		#region Properties
		public string Tag {
			get => Marshal.PtrToStringAnsi(TagComponentGetTag(componentPtr));
			set => TagComponentSetTag(componentPtr, value);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern System.IntPtr TagComponentGetTag(System.IntPtr comp);

		[DllImport("EngineCore")]
		static extern void TagComponentSetTag(System.IntPtr comp, string tag);
		#endregion
	}
}
