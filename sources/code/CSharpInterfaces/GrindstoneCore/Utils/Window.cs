using Grindstone.Math;
using System.Runtime.InteropServices;

namespace Grindstone
{
	public class Window
	{
		#region Enums
		public enum FullscreenMode {
			Windowed,
			Borderless,
			Fullscreen
		}
		#endregion

		#region Private Fields
		private System.IntPtr windowIntPtr;
		#endregion

		#region Properties
		public static Window Current => new Window(WindowGetCurrentWindow());

		public FullscreenMode Fullscreen {
			set => WindowSetFullscreenMode(windowIntPtr, value);
		}

		public Rect WindowRect {
			get
			{
				WindowGetRect(windowIntPtr, out uint left, out uint top, out uint right, out uint bottom);
				return new Rect(left, top, right, bottom);
			}
		}

		public Float2 Size {
			get
			{
				WindowGetSize(windowIntPtr, out uint width, out uint height);
				return new Float2(width, height);
			}
			set => WindowSetSize(windowIntPtr, (uint)value.x, (uint)value.y);
		}

		public Float2 Position {
			get
			{
				WindowGetPosition(windowIntPtr, out int x, out int y);
				return new Float2(x, y);
			}
			set => WindowSetPosition(windowIntPtr, (int)value.x, (int)value.y);
		}

		public bool IsFocused {
			get => WindowGetIsFocused(windowIntPtr);
			set => WindowSetIsFocused(windowIntPtr, value);
		}

		public bool IsMinimized => WindowGetIsMinimized(windowIntPtr);

		public string Title {
			get => WindowGetTitle(windowIntPtr);
			set => WindowSetTitle(windowIntPtr, value);
		}
		#endregion

		#region Public Methods
		public Window(System.IntPtr p) {
			windowIntPtr = p;
		}

		public void Close() {
			WindowClose(windowIntPtr);
		}

		public void Show() {
			WindowShow(windowIntPtr);
		}

		public void Hide() {
			WindowHide(windowIntPtr);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern System.IntPtr WindowGetCurrentWindow();

		[DllImport("EngineCore")]
		private static extern void WindowClose(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern void WindowShow(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern void WindowHide(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern void WindowSetFullscreenMode(System.IntPtr windowPtr, FullscreenMode value);

		[DllImport("EngineCore")]
		private static extern void WindowGetRect(System.IntPtr windowPtr, out uint left, out uint top, out uint right, out uint bottom);

		[DllImport("EngineCore")]
		private static extern void WindowGetSize(System.IntPtr windowPtr, out uint width, out uint height);

		[DllImport("EngineCore")]
		private static extern void WindowSetSize(System.IntPtr windowPtr, uint width, uint height);

		[DllImport("EngineCore")]
		private static extern void WindowGetPosition(System.IntPtr windowPtr, out int x, out int y);

		[DllImport("EngineCore")]
		private static extern void WindowSetPosition(System.IntPtr windowPtr, int x, int y);

		[DllImport("EngineCore")]
		private static extern bool WindowGetIsFocused(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern void WindowSetIsFocused(System.IntPtr windowPtr, bool isFocused);

		[DllImport("EngineCore")]
		private static extern bool WindowGetIsMinimized(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern string WindowGetTitle(System.IntPtr windowPtr);

		[DllImport("EngineCore")]
		private static extern void WindowSetTitle(System.IntPtr windowPtr, string title);
		#endregion
	}
}
