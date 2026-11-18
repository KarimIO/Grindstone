using Grindstone.Math;
using System.Runtime.InteropServices;

namespace Grindstone.Input
{
	public static class InputManager
	{
		#region Static Methods
		public static bool IsKeyDown(KeyboardKey keyboardKey)
		{
			return InputManagerIsKeyDown((int)keyboardKey) != 0;
		}

		public static bool IsMouseButtonDown(MouseButton mouseButton)
		{
			return InputManagerIsMouseButtonDown((int)mouseButton) != 0;
		}

		public static Float2 MousePosition
		{
			get => InputManagerGetMousePos();
			set => InputManagerSetMousePos(value);
		}

		public static bool IsWindowFocused => InputManagerGetIsWindowFocused() != 0;

		public static bool IsMouseRawMotion
		{
			get => InputManagerGetMouseIsRawMotion() != 0;
			set => InputManagerSetMouseIsRawMotion((byte)(value ? 1 : 0));
		}

		public static CursorMode CursorMode
		{
			get => (CursorMode)InputManagerGetCursorMode();
			set => InputManagerSetCursorMode((byte)value);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern byte InputManagerGetCursorMode();

		[DllImport("EngineCore")]
		private static extern void InputManagerSetCursorMode(byte cursorMode);

		[DllImport("EngineCore")]
		private static extern byte InputManagerGetMouseIsRawMotion();

		[DllImport("EngineCore")]
		private static extern void InputManagerSetMouseIsRawMotion(byte isRawMotion);

		[DllImport("EngineCore")]
		private static extern byte InputManagerGetIsWindowFocused();

		[DllImport("EngineCore")]
		private static extern byte InputManagerIsKeyDown(int keyboardKey);

		[DllImport("EngineCore")]
		private static extern byte InputManagerIsMouseButtonDown(int mouseButton);

		[DllImport("EngineCore")]
		private static extern Float2 InputManagerGetMousePos();

		[DllImport("EngineCore")]
		private static extern void InputManagerSetMousePos(Float2 mousePos);
		#endregion
	}
}
