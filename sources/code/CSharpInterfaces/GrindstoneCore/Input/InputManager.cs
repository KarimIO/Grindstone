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
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern bool InputManagerIsKeyDown(int keyboardKey);

		[DllImport("EngineCore")]
		private static extern bool InputManagerIsMouseButtonDown(int mouseButton);
		#endregion
	}
}
