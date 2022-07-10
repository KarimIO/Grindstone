using System.Runtime.InteropServices;

namespace Grindstone {
	public static class SceneManager {
		#region Static Methods
		public static Scene GetActiveScene() {
			var scenePtr = SceneManagerGetActiveScene();

			return new Scene(scenePtr);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern System.IntPtr SceneManagerGetActiveScene();
		#endregion
	}
}
