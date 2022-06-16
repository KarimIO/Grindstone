using System.Runtime.InteropServices;

namespace Grindstone {
	public class SceneManager {
		public static Scene GetActiveScene() {
			var scenePtr = SceneManagerGetActiveScene();

			return new Scene(scenePtr);
		}

		[DllImport("EngineCore")]
		static extern System.IntPtr SceneManagerGetActiveScene();
	}
}
