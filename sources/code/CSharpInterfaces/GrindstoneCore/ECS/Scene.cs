using System.Runtime.InteropServices;

namespace Grindstone {
	public struct Scene {
		#region Private Fields
		public System.IntPtr sceneIntPtr;
		#endregion

		#region Public Methods
		public Scene(System.IntPtr p) {
			sceneIntPtr = p;
		}

		public bool IsValidScene() {
			return sceneIntPtr != System.IntPtr.Zero;
		}

		public override string ToString() {
			if (!IsValidScene()) {
				Logger.PrintError("Scene::ToString() - Calling on invalid scene.");
				return null;
			}

			return $"Scene({GetName()})";
		}

		public string GetName() {
			if (!IsValidScene()) {
				Logger.PrintError("Scene::GetName() - Calling on invalid scene.");
				return null;
			}

			return Marshal.PtrToStringAnsi(SceneGetName(sceneIntPtr));
		}

		public string GetPath() {
			if (!IsValidScene()) {
				Logger.PrintError("Scene::GetPath() - Calling on invalid scene.");
				return null;
			}

			return Marshal.PtrToStringAnsi(SceneGetPath(sceneIntPtr));
		}

		public Entity CreateEntity() {
			if (!IsValidScene()) {
				Logger.PrintError("Scene::CreateEntity() - Calling on invalid scene.");
				return new Entity(uint.MaxValue, new Scene(System.IntPtr.Zero));
			}

			uint entityHandle = SceneCreateEntity(sceneIntPtr);
			return new Entity(entityHandle, this);
		}

		public void DestroyEntity(uint entityHandle) {
			if (!IsValidScene()) {
				Logger.PrintError("Scene::DestroyEntity(int) - Calling on invalid scene.");
				return;
			}

			SceneDestroyEntity(sceneIntPtr, entityHandle);
		}

		public static void DestroyEntity(Entity entity) {
			if (entity.entityHandle == Entity.InvalidHandle || !entity.scene.IsValidScene()) {
				Logger.PrintError("Scene::DestroyEntity(Entity) - Calling on invalid scene.");
				return;
			}

			SceneDestroyEntity(entity.scene.sceneIntPtr, entity.entityHandle);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern uint SceneCreateEntity(System.IntPtr scene);

		[DllImport("EngineCore")]
		static extern void SceneDestroyEntity(System.IntPtr scene, uint entityHandle);

		[DllImport("EngineCore")]
		static extern System.IntPtr SceneGetName(System.IntPtr scene);

		[DllImport("EngineCore")]
		static extern System.IntPtr SceneGetPath(System.IntPtr scene);
		#endregion
	}
}
