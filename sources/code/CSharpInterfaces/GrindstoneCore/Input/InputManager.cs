using Grindstone.Math;
using System.Runtime.InteropServices;

namespace Grindstone.Input {
	public static class InputManager {
		#region Static Methods
		public static bool IsKeyDown(KeyboardKey keyboardKey) {
			return InputManagerIsKeyDown((int)keyboardKey);
		}

		public static bool IsMouseButtonDown(MouseButton mouseButton) {
			return InputManagerIsMouseButtonDown((int)mouseButton);
		}

		public static Float2 MousePosition {
			get => InputManagerGetMousePos();
			set => InputManagerSetMousePos(value);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern bool InputManagerIsKeyDown(int keyboardKey);

		[DllImport("EngineCore")]
		private static extern bool InputManagerIsMouseButtonDown(int mouseButton);

		[DllImport("EngineCore")]
		private static extern Float2 InputManagerGetMousePos();

		[DllImport("EngineCore")]
		private static extern void InputManagerSetMousePos(Float2 mousePos);
		#endregion
	}
}
